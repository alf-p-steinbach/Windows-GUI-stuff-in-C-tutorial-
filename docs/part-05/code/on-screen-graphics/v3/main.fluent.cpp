# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::*

#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

namespace gdi = winapi::gdi;
using   gdi::Brush, gdi::Pen, gdi::Dc, gdi::Screen_dc;
using   std::string_view, std::size;

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    
    // Clear the background to blue.
    FillRect( canvas + CreateSolidBrush( blue ), &area, 0 );

    // Draw a yellow circle filled with orange.
    Ellipse(
        canvas + CreatePen( PS_SOLID, 1, yellow ) + CreateSolidBrush( orange ),
        area.left, area.top, area.right, area.bottom
        );
}

auto main() -> int
{
    draw_on( Screen_dc(), RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
