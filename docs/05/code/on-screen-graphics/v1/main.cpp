# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

// COLORREF is 32-bit unsigned.
namespace color {
    constexpr COLORREF  orange      = RGB( 0xFF, 0x80, 0x20 );
    constexpr COLORREF  yellow      = RGB( 0xFF, 0xFF, 0x20 );
    constexpr COLORREF  blue        = RGB( 0x00, 0x00, 0xFF );
}  // namespace color

auto main() -> int
{
    constexpr auto  no_window   = HWND( 0 );
    constexpr auto  area        = RECT{ 10, 10, 10 + 400, 10 + 400 };
    
    const HDC canvas = GetDC( no_window );
        // Fill the background with blue.
        const HBRUSH blue_brush = CreateSolidBrush( color::blue );
            FillRect( canvas, &area, blue_brush );
        DeleteObject( blue_brush );

        // Draw a yellow circle filled with orange.
        const HBRUSH    orange_brush                = CreateSolidBrush( color::orange );
            const HGDIOBJ   original_brush          = SelectObject( canvas, orange_brush );
                const HPEN      yellow_pen          = CreatePen( PS_SOLID, 1, color::yellow );
                    const HGDIOBJ   original_pen    = SelectObject( canvas, yellow_pen );
                        Ellipse( canvas, area.left, area.top, area.right, area.bottom );
                    SelectObject( canvas, original_pen );
                DeleteObject( yellow_pen );
            SelectObject( canvas, original_brush );
        DeleteObject( orange_brush );
    ReleaseDC( no_window, canvas );
}
