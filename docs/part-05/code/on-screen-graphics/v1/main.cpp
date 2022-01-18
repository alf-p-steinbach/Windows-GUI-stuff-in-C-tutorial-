# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

auto main() -> int
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  no_window   = HWND( 0 );
    constexpr auto  area        = RECT{ 10, 10, 10 + 400, 10 + 400 };
    
    const HDC canvas = GetDC( no_window );

    // Fill the background with blue.
    const HBRUSH blue_brush = CreateSolidBrush( blue );
    FillRect( canvas, &area, blue_brush );
    DeleteObject( blue_brush );

    // Draw a yellow circle filled with orange.
    const HBRUSH    orange_brush    = CreateSolidBrush( orange );
    const HGDIOBJ   original_brush  = SelectObject( canvas, orange_brush );
    const HPEN      yellow_pen      = CreatePen( PS_SOLID, 1, yellow );
    const HGDIOBJ   original_pen    = SelectObject( canvas, yellow_pen );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
    SelectObject( canvas, original_pen );
    DeleteObject( yellow_pen );
    SelectObject( canvas, original_brush );
    DeleteObject( orange_brush );

    ReleaseDC( no_window, canvas );
}
