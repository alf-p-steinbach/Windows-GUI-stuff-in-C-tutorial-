# // Source encoding: UTF-8 with BOM (p is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/gdi.hpp>
#include "dragon_curve.hpp"
namespace gdi = winapi::gdi;
namespace dr = dragon_curve;

#include <vector>
using   std::vector;

#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif

auto main() -> int
{
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  no_window   = HWND( 0 );

    vector<POINT> points;
    dr::generate(
        11,
        [&]( dr::Point pt ){ points.push_back( POINT{ pt.x, pt.y } ); },
        3
        );
    
    const auto canvas       = gdi::Window_dc( no_window );
    const auto blue_brush   = gdi::Object_( CreateSolidBrush( blue ) );
    const auto orange_brush = gdi::Object_( CreateSolidBrush( orange ) );
    const auto white_pen    = ::GetStockObject( WHITE_PEN );
    const auto white_brush  = ::GetStockObject( WHITE_BRUSH );

    printf( "%d points\n", (int) points.size() );

    {
        const auto _ = gdi::Selection( canvas, orange_brush );
        ::Rectangle( canvas, 10, 10, 10 + 400, 10 + 400 );
    }
    
    { // Using the blue brush.
        const auto _1 = gdi::Selection( canvas, blue_brush );
        const auto _2 = gdi::Selection( canvas, white_pen );
        ::SetViewportOrgEx( canvas, 0, 0, nullptr );
        ::Ellipse( canvas, 30, 10, 10 + 400, 10 + 400 );
        ::GdiFlush();
        ::SetViewportOrgEx( canvas, 150, 180, nullptr );
        ::Polyline( canvas, points.data(), static_cast<int>( points.size() ) );
    }
}
