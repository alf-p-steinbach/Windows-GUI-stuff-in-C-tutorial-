#include "resources.h"                  // Resource identifier macros.

#include <winapi/gdi/color_names.hpp>   // winapi::gdi::color_names::*
#include <winapi/gui/util.hpp>          // winapi::gui::*, winapi::kernel::*

namespace color = winapi::gdi::color_names;
namespace wg    = winapi::gui;
namespace wk    = winapi::kernel;

#include <stdlib.h>     // EXIT_...
#include <math.h>

#include <optional>
#include <tuple>        // std::ignore
using   std::optional,
        std::ignore;

namespace calc {
    const double pi = acos( -1 );
    
    struct Angle
    {
        double value;
        auto as_float() const -> auto { return static_cast<float>( value ); }
    };

    struct Radians: Angle {};
    struct Degrees: Angle {};
    
    auto to_radians( const Degrees v ) -> Radians { return {pi*v.value/180}; }
    auto to_degrees( const Radians v ) -> Degrees { return {180*v.value/pi}; }
}  // namespace calc

// The `bounds` argument specifies the ellipse boundary rectangle /before/ it's rotated.
void draw_ellipse( const HDC canvas, const RECT& bounds, const calc::Radians angle )
{
    // Rotate clockwise because the coordinate mapping mode is MM_TEXT.
    const auto [w, h] = SIZE{ bounds.right - bounds.left, bounds.bottom - bounds.top };
    const auto translation = SIZE{ bounds.left + w/2, bounds.top + h/2 };
    const float c = cosf( angle.as_float() );
    const float s = sinf( angle.as_float() );
    XFORM transform =
    {
        c, s,       // eM11, eM12
        -s, c,      // eM21, eM22
        float( translation.cx ), float( translation.cy )    // eDx, eDy
    };
    const int original_mode = SetGraphicsMode( canvas, GM_ADVANCED );
    assert( original_mode == GM_COMPATIBLE );               // Ensures MM_TEXT mapping mode.
    SetWorldTransform(canvas, &transform );
    Ellipse(
        canvas,
        bounds.left - translation.cx, bounds.top - translation.cy,
        bounds.right - translation.cx, bounds.bottom - translation.cy
        );
    ModifyWorldTransform( canvas, nullptr, MWT_IDENTITY );  // Reset the transform, so that
    SetGraphicsMode( canvas, original_mode );               // the mode can be reset.
}

void draw_on( const HDC canvas, const RECT& area )
{
    // Clear the background to blue.
    SetDCBrushColor( canvas, color::blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, color::yellow );
    SetDCBrushColor( canvas, color::orange );
    
    const auto degrees_per_second = double( 60 );
    const auto seconds = double( GetTickCount() )/1000;
    const auto angle = calc::to_radians( calc::Degrees{ degrees_per_second*seconds } );
    draw_ellipse( canvas, area, angle );
}

auto dc_colors_enabled( const HDC dc )
    -> HDC
{
    SelectObject( dc, GetStockObject( DC_PEN ) );
    SelectObject( dc, GetStockObject( DC_BRUSH ) );
    return dc;
}

void paint( const HWND window, const HDC dc )
{
    RECT client_rect;
    GetClientRect( window, &client_rect );

    #ifndef NO_DOUBLEBUFFERING_PLEASE
        const auto [width, height] = SIZE{ client_rect.right, client_rect.bottom };

        // Create an off-screen DC with a compatible bitmap, for double-buffering.
        // See <url: http://www.catch22.net/tuts/win32/flicker-free-drawing> for some background.
        const HDC memory_dc = dc_colors_enabled( CreateCompatibleDC( dc ) );
        const HBITMAP bitmap = CreateCompatibleBitmap( dc, width, height );
        const HGDIOBJ original_bitmap = SelectObject( memory_dc, bitmap );

            draw_on( memory_dc, client_rect );
            BitBlt( dc, 0, 0, width, height, memory_dc, 0, 0, SRCCOPY );

        SelectObject( memory_dc, original_bitmap );
        DeleteObject( bitmap );
        DeleteDC( memory_dc);
    #else
        draw_on( dc_colors_enabled( dc ), client_rect );
    #endif
}

namespace on_wm {
    void close( const HWND window )
    {
        EndDialog( window, IDOK );
    }

    auto erasebkgnd( const HWND window, const HDC dc )
        -> bool
    {
        ignore = window;  ignore = dc;
        return true;        // Prevents background erasure which can cause flicker.
    }

    auto initdialog( const HWND window, const HWND focus, const LPARAM ell_param )
        -> bool
    {
        ignore = focus; ignore = ell_param;

        wg::remove_topmost_style_for( window );
        wg::set_client_area_size( window, 400, 400 );

        const int timer_id = 1;             // Arbitrary.
        const int n_millisecs = 1000/50;    // Sufficient for smooth animation.
        SetTimer( window, timer_id, n_millisecs, nullptr );
        return true;    // `true` sets focus to the `focus` control.
    }

    void paint( const HWND window )
    {
        PAINTSTRUCT info;
        if( const HDC dc = BeginPaint( window, &info ) ) {
            ::paint( window, dc );
        }
        EndPaint( window, &info );  // Docs say this must be called for each BeginPaint.
    }
    
    void size( const HWND window, const UINT state, const int new_width, const int new_height )
    {
        ignore = state; ignore = new_width; ignore = new_height;
        const auto the_whole_client_area = nullptr;
        InvalidateRect( window, the_whole_client_area, not "erase the background" );
    }

    void timer( const HWND window, const UINT id )
    {
        ignore = id;
        InvalidateRect( window, nullptr, false );
        // RedrawWindow( window, nullptr, nullptr, RDW_NOERASE | RDW_UPDATENOW );
    }
}  // namespace on_wm

auto CALLBACK dialog_message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    optional<INT_PTR> result;

    #define HANDLE_WM( name, handler_func ) \
        HANDLE_WM_##name( window, w_param, ell_param, handler_func )
    switch( msg_id ) {
        case WM_CLOSE:      result = HANDLE_WM( CLOSE, on_wm::close ); break;
        case WM_ERASEBKGND: result = HANDLE_WM( ERASEBKGND, on_wm::erasebkgnd ); break;
        case WM_INITDIALOG: result = HANDLE_WM( INITDIALOG, on_wm::initdialog ); break;
        case WM_PAINT:      result = HANDLE_WM( PAINT, on_wm::paint ); break;
        case WM_SIZE:       result = HANDLE_WM( SIZE, on_wm::size ); break;
        case WM_TIMER:      result = HANDLE_WM( TIMER, on_wm::timer ); break;
    }
    #undef HANDLE_WM

    // `false` => Didn't process the message, want default processing.
    return (result? SetDlgMsgResult( window, msg_id, result.value() ) : false);
}

auto main() -> int
{
    // The `DialogBox` return value is misdocumented per 2022, but is like `DialogBoxParam`.
    const auto dialogbox_result = DialogBox(
        wk::this_exe,
        wk::Resource_id{ IDD_MAIN_WINDOW }.as_pseudo_ptr(),
        HWND(),             // Parent window, a zero handle is "no parent".
        &dialog_message_handler
        );
    return (dialogbox_result <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
