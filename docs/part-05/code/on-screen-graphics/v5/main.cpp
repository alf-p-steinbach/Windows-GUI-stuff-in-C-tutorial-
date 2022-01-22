﻿# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
#include <winapi/gdi-text-display.hpp>      // winapi::gdi::draw_text
#include <winapi/gui-util.hpp>              // winapi::gui::std_gui_font
#include <winapi/ole2.hpp>                  // winapi::ole2::Library_usage

#include <stdio.h>          // stderr, fprintf
#include <stdlib.h>         // EXIT_...

#include <stdexcept>        // std::exception
#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

namespace gdi   = winapi::gdi;
namespace ole2  = winapi::ole2;
using   std::exception, std::string_view, std::size;

void draw_on( const HDC canvas, const RECT& area )
{
    constexpr auto  white       = COLORREF( RGB( 0xFF, 0xFF, 0xFF ) );  (void) white;   // Unused.
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  black       = COLORREF( RGB( 0, 0, 0 ) );
    
    // Clear the background to blue.
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
    
    // Draw some international (English, Russian, Chinese, Norwegian) text.
    constexpr auto text = string_view( "Every 日本国 кошка loves\nNorwegian blåbærsyltetøy!" );
    SetTextColor( canvas, black );          // This is also the default, but making it explicit.
    auto text_rect = RECT{ area.left + 40, area.top + 150, area.right, area.bottom };
    gdi::draw_text( canvas, text, text_rect );
}

void init( const HDC canvas )
{
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SetBkMode( canvas, TRANSPARENT );       // Don't fill in the background of text, please.
    SelectObject( canvas, winapi::gui::std_gui_font.handle );
}

void cpp_main()
{
    assert( GetACP() == CP_UTF8 );
    constexpr auto no_window = HWND( 0 );
    const int width     = 400;
    const int height    = 400;

    const HDC       screen          = GetDC( no_window );
    const HDC       canvas          = CreateCompatibleDC( screen );
    const HBITMAP   bitmap          = CreateCompatibleBitmap( screen, width, height );
    const HGDIOBJ   original_bitmap = SelectObject( canvas, bitmap );

    init( canvas );
    draw_on( canvas, RECT{ 0, 0, width, height } );

    SelectObject( canvas, original_bitmap );
    DeleteObject( bitmap );
    DeleteDC( canvas );
    ReleaseDC( no_window, canvas );
}

auto main() -> int
{
    try {
        const ole2::Library_usage _;
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
    }
    return EXIT_FAILURE;
}
