# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::*

#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

namespace gdi = winapi::gdi;
using   gdi::Brush, gdi::Pen, gdi::Dc, gdi::Screen_dc;

template< class... Args > void evaluate( const Args&... ) {}

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    
    // Clear the background to blue.
    FillRect( canvas,  &area, Brush( CreateSolidBrush( blue ) ) );

    { // Draw a yellow circle filled with orange.
        const auto pen      = Pen( CreatePen( PS_SOLID, 1, yellow ) );
        const auto brush    = Brush( CreateSolidBrush( orange ) );

        const auto s1       = Dc::Selection( canvas, pen );
        const auto s2       = Dc::Selection( canvas, brush );
        Ellipse( canvas, area.left, area.top, area.right, area.bottom );
    }
}

auto main() -> int
{
    draw_on( Screen_dc(), RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
