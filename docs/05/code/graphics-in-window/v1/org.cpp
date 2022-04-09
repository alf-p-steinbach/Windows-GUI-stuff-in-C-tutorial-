# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <windows/with_exceptions_reported.hpp>
#include <winapi/gdi/color_names.hpp>

#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::(Dc, Bitmap_dc, `+`)
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32
#include <winapi/gdi/bitmap-util.hpp>       // winapi::gdi::save_to

#include <stdlib.h>     // EXIT_..., system
#include <stdio.h>      // fprintf

#include <functional>   // std::ref
#include <stdexcept>    // std::exception
#include <string>       // std::string

namespace gdi   = winapi::gdi;
namespace color = gdi::color_names;

using   gdi::Dc, gdi::Bitmap_32, gdi::Bitmap_dc, gdi::save_to;
using   std::exception, std::string;

const auto s = string();    // For string concatenation.

void draw_on( const HDC canvas, const RECT& area )
{
    // Clear the background to blue.
    SetDCBrushColor( canvas, color::blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, color::yellow );
    SetDCBrushColor( canvas, color::orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
}

void cpp_main()
{
    const auto [w, h] = SIZE{ 400, 400 };
    auto image = Bitmap_32( w, h );

    draw_on( Bitmap_dc( image ), RECT{ 0, 0, w, h } );

    const auto filename = string( "image-saving-result.bmp" );
    gdi::save_to( filename, image );
    #ifndef QUIET_PLEASE
        auto cmd = ::s + "start \"\" " + filename + "\"";   // “start” for non-blocking command.
        system( cmd.c_str() );  // Open file in “.bmp”-associated program, e.g. an image viewer.
    #endif
}

auto main( int, char** cmd_line_parts )
    -> int
{ return windows::with_exceptions_reported( cpp_main, cmd_line_parts[0] ); }
