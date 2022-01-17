# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <compiler/msvc/Assertion_reporting_fix.auto.hpp>

#include "c_curve.hpp"
#include <stdio.h>              // printf
#include <string.h>             // strlen
#include <winapi/gdi.hpp>       // Window_dc, Object_, Selection
#include <winapi/util.hpp>      // std_gui_font
namespace gdi   = winapi::gdi;
namespace wu    = winapi::util;
using   std::vector;

auto main() -> int
{
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  no_window   = HWND( 0 );

    const auto canvas           = gdi::Window_dc( no_window );
    const auto blue_brush       = gdi::Object_( CreateSolidBrush( blue ) );
    const auto orange_brush     = gdi::Object_( CreateSolidBrush( orange ) );
    const auto white_pen        = GetStockObject( WHITE_PEN );      // No cleanup because
    const auto white_brush      = GetStockObject( WHITE_BRUSH );    // ... system objects.
    (void) white_brush; struct white_brush;                         // Well, it's not used.

    const auto area = RECT{ 10, 10, 10 + 400, 10 + 400 };
    FillRect( canvas, &area, orange_brush );                        // Consistent background.
    
    const vector<POINT> points = c_curve::as_vector_of_<POINT>( 11, 3 );
    printf( "%d points.\n", (int) points.size() );
    { // Display the curve.
        const auto _1 = gdi::Selection( canvas, blue_brush );
        const auto _2 = gdi::Selection( canvas, white_pen );
        Ellipse( canvas, area.left + 3, area.top + 3, area.right - 3, area.bottom - 3 );
        SetViewportOrgEx( canvas, 150, 180, nullptr );      // Empirically determined offsets.
        Polyline( canvas, points.data(), static_cast<int>( points.size() ) );
        SetViewportOrgEx( canvas, 0, 0, nullptr );
    }

    { // Add explanatory text.
        const auto& s = "The C curve!";     // Only ASCII, because TextOut isn't UTF-8 aware.
        #ifndef DEFTEXT
            SetBkMode( canvas, TRANSPARENT );
            SetTextColor( canvas, yellow );
            const auto _ = gdi::Selection( canvas, wu::std_gui_font.handle );
        #else
            (void) yellow;
        #endif
        TextOut( canvas, 80, 220, s, static_cast<int>( strlen( s ) ) );
    }

    // At this point the brushes and canvas are destroyed automatically via RAII.
}
