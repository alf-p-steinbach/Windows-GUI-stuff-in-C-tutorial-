# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

using   std::string_view, std::size;

template< class T > auto int_size( const T& o ) -> int { return static_cast<int>( size( o ) ); }

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
    
    // Draw some international text. Note: non-ASCII UTF-8 characters are incorrectly rendered.
    constexpr auto text = string_view( "Every 日本国 кошка loves\r\nNorwegian Blåbærsyltetøy!" );
    auto text_rect = RECT{ area.left + 40, area.top + 150, area.right, area.bottom };   // Not `const`.
    SetTextColor( canvas, black );              // This is also the default, but making it explicit.
    SetBkMode( canvas, TRANSPARENT );
    DrawText( canvas, text.data(), int_size( text ), &text_rect, DT_LEFT | DT_TOP | DT_NOPREFIX );
}

auto main() -> int
{
    assert( GetACP() == CP_UTF8 );
    constexpr auto  no_window   = HWND( 0 );

    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    
    draw_on( canvas, RECT{ 10, 10, 10 + 400, 10 + 400 } );

    ReleaseDC( no_window, canvas );
}
