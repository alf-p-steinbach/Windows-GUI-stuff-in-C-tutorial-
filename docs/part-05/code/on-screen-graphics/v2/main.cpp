# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

#include <type_traits>
using std::is_same_v;

static_assert( is_of_type_<COLORREF>( RGB(0,0,0) ) );   // RGB() → COLORREF 
static_assert( is_same_v<COLORREF, DWORD> );            // COLORREF is 32-bit unsigned.
namespace color {
    constexpr auto  orange      = RGB( 0xFF, 0x80, 0x20 );
    constexpr auto  yellow      = RGB( 0xFF, 0xFF, 0x20 );
    constexpr auto  blue        = RGB( 0, 0, 0xFF );
}  // namespace color

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

auto main() -> int
{
    constexpr auto  no_window   = HWND( 0 );

    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );

    draw_on( canvas, RECT{ 10, 10, 10 + 400, 10 + 400 } );

    ReleaseDC( no_window, canvas );
}
