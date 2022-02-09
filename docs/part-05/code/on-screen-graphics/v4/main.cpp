# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::*

namespace gdi = winapi::gdi;
using   gdi::Dc, gdi::Screen_dc;

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  black       = COLORREF( 0 );
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    
    SetBkMode( canvas, OPAQUE );
    SetBkColor( canvas, black );        // The fill color of the `FillRect` call below.
    FillRect( canvas + CreateHatchBrush( HS_DIAGCROSS, blue ), &area, 0 );

    Ellipse(
        canvas + CreatePen( PS_SOLID, 1, yellow ) + CreateSolidBrush( orange ),
        area.left, area.top, area.right, area.bottom
        );
}

auto main() -> int
{
    draw_on( Screen_dc(), RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
