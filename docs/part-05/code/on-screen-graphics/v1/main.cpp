# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

auto main() -> int
{
    constexpr auto  red         = COLORREF( RGB( 0xFF, 0, 0 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  no_window   = HWND( 0 );
    
    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SelectObject( canvas, GetStockObject( DC_PEN ) );

    SetDCBrushColor( canvas, red );
    SetDCPenColor( canvas, yellow );
    Ellipse( canvas, 10, 10, 10 + 400, 10 + 400 );

    ReleaseDC( no_window, canvas );
}
