# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::*
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32

#include <utility>      // std::pair

namespace gdi = winapi::gdi;
using   gdi::Dc, gdi::Screen_dc, gdi::Bitmap_32, gdi::Bitmap_dc;
using   std::ref, std::pair;

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

auto main() -> int
{
    const auto [w, h] = pair( 400, 400 );
    auto image = Bitmap_32( w, h );
    draw_on( Bitmap_dc( ref( image ) ), RECT{ 10, 10, 10 + w, 10 + h } );
}
