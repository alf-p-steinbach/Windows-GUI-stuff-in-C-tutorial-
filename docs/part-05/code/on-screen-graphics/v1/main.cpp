#include <wrapped-winapi/windows-h.hpp>

auto main() -> int
{
    constexpr auto  red         = COLORREF( RGB( 0xFF, 0, 0 ) );
    constexpr auto  no_window   = HWND( 0 );
    
    const HDC canvas = GetDC( no_window );
    {
        const HBRUSH red_brush = CreateSolidBrush( red );
        {
            const HGDIOBJ original_brush = SelectObject( canvas, red_brush );
            {
                Ellipse( canvas, 10, 10, 10 + 400, 10 + 400 );
            }
            SelectObject( canvas, original_brush );
        }
        DeleteObject( red_brush );
    }
    ReleaseDC( no_window, canvas );
}
