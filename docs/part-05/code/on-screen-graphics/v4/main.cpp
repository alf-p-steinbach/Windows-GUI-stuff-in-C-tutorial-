# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::(Dc, Bitmap_dc, `+`)
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32
#include <winapi/gdi/bitmap-util.hpp>       // winapi::gdi::save_to

#include <stdlib.h>     // EXIT_..., system
#include <stdio.h>      // fprintf

#include <functional>   // std::ref
#include <stdexcept>    // std::exception
#include <string>       // std::string

namespace gdi = winapi::gdi;
using   gdi::Dc, gdi::Bitmap_32, gdi::Bitmap_dc, gdi::save_to;
using   std::ref, std::exception, std::string;
const auto s = string();    // For string concatenation.

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    
    FillRect( canvas + CreateSolidBrush( blue ), &area, 0 );
    Ellipse(
        canvas + CreatePen( PS_SOLID, 1, yellow ) + CreateSolidBrush( orange ),
        area.left, area.top, area.right, area.bottom
        );
}

void cpp_main()
{
    const auto [w, h] = SIZE{ 400, 400 };
    auto image = Bitmap_32( w, h );
    draw_on( Bitmap_dc( ref( image ) ), RECT{ 0, 0, w, h } );
    const auto filename = string( "image-saving-result.bmp" );
    gdi::save_to( filename, image );
    
    auto cmd = ::s + "start \"\" " + filename + "\"";   // “start” for non-blocking command.
    system( cmd.c_str() );  // Open file in “.bmp”-associated program, e.g. an image viewer.
}

auto main( int, char** args ) -> int
{
    const auto error_box = []( const string& title, string const& text )
    {
        const auto as_errorbox  = MB_ICONERROR | MB_SYSTEMMODAL;    // Sys-modal for Win 11.
        MessageBox( 0, text.c_str(), title.c_str(), as_errorbox );
    };

    using std::exception;
    try {
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
        error_box( ::s + args[0] + " failed:", ::s + "Because:\n" + x.what() );
    }
    return EXIT_FAILURE;
}
