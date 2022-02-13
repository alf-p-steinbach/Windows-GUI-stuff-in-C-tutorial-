# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi"). v100
#include <cpp/util.hpp>                     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/com/failure-checking.hpp>  // winapi::com::operator>>
#include <winapi/com/Ptr_.hpp>              // winapi::com::Ptr_
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32
#include <winapi/gdi/bitmap-util.hpp>       // winapi::gdi::save_to
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::(Screen_dc, Memory_dc)
#include <winapi/gui/util.hpp>              // winapi::gui::std_gui_font
#include <winapi/ole/B_string.hpp>          // winapi::ole::B_string
#include <winapi/ole/Library_usage.hpp>     // winapi::ole::Library_usage
#include <winapi/ole/picture-util.hpp>      // winapi::ole::picture_from

// #include <shlwapi.h>        // SHCreateStreamOnFileEx
#include <stdio.h>          // fprintf
#include <stdlib.h>         // EXIT_...

#include <stdexcept>        // std::exception&
#include <string>           // std::string
#include <string_view>      // std::(string_view, wstring_view)
#include <utility>          // std::ref

namespace cu  = cpp::util;
namespace com = winapi::com;
namespace ole = winapi::ole;
namespace gdi = winapi::gdi;
using   cu::hopefully, cu::fail, cu::success, cu::No_copying, cu::int_size, cu::Const_;
using   com::failure_checking::operator>>;
using   ole::save_to;
using   gdi::Bitmap_32, gdi::save_to;
using   std::string,
        std::string_view, std::wstring_view,
        std::ref;

void display_graphics_on( const HDC canvas )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  area        = RECT{ 0, 0, 400, 400 };
    
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );   // `0` works for me, but should perhaps be the DC brush.

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
}

void init( const HDC canvas )
{
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SetBkMode( canvas, TRANSPARENT );       // Don't fill in the background of text, please.
    SelectObject( canvas, winapi::gui::std_font.handle() );
}

void display_graphics()
{
    const int width     = 400;
    const int height    = 400;
    auto bitmap = Bitmap_32( width, height );
    const auto dc = winapi::gdi::Bitmap_dc( ref( bitmap ) );
    init( dc.handle() );
    display_graphics_on( dc.handle() );
    //BitBlt( Screen_dc().handle, 15, 15, width, height, dc.handle(), 0, 0, SRCCOPY );
    save_to( "generated-image.bmp", bitmap );
}

auto main( int, char** args ) -> int
{
    using   std::exception;
    try {
        const ole::Library_usage _;
        display_graphics();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
        const auto as_errorbox  = MB_ICONERROR | MB_SYSTEMMODAL;
        const auto title        = string() + args[0] + " failed:";
        const auto text         = string() + "Because:\n" + x.what();
        MessageBox( 0, text.c_str(), title.c_str(), as_errorbox );
    }
    return EXIT_FAILURE;
}
