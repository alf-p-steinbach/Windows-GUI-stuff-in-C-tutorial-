#include "resources.h"                  // Resource identifier macros.

#include <winapi/gdi/color_names.hpp>   // winapi::gdi::color_names::*
#include <winapi/gui/util.hpp>          // winapi::gui::*, winapi::kernel::*

namespace color = winapi::gdi::color_names;
namespace wg    = winapi::gui;
namespace wk    = winapi::kernel;

#include <stdlib.h>     // EXIT_...

#include <optional>
using   std::optional;

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

void paint( const HWND window, const HDC dc )
{
    RECT client_rect;
    GetClientRect( window, &client_rect );
    draw_on( dc, client_rect );
}

void set_client_area_size( const HWND window, const int width, const int height )
{
    RECT r = {0, 0, width, height};                 // Desired client area.

    const LONG window_style = GetWindowLong( window, GWL_STYLE );
    const bool has_menu = (GetMenu( window ) != 0);
    AdjustWindowRect( &r, window_style, has_menu ); // Find window size for given client rect.

    SetWindowPos( window, HWND(), 0, 0, r.right, r.bottom, SWP_NOMOVE|SWP_NOZORDER );
}

namespace on_wm {
    void close( const HWND window )
    {
        EndDialog( window, IDOK );
    }

    auto initdialog( const HWND window, const HWND /*focus*/, const LPARAM /*ell_param*/ )
        -> bool
    {
        wg::remove_topmost_style_for( window );
        set_client_area_size( window, 400, 400 );
        
        return true;    // `true` sets focus to the `focus` control.
    }

    void paint( const HWND window )
    {
        PAINTSTRUCT info;
        if( const HDC dc = BeginPaint( window, &info ) ) {
            SelectObject( dc, GetStockObject( DC_PEN ) );
            SelectObject( dc, GetStockObject( DC_BRUSH ) );
            ::paint( window, dc );
        }
        EndPaint( window, &info );  // Docs say this must be called for each BeginPaint.
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
        case WM_CLOSE:      result = HANDLE_WM( CLOSE,      on_wm::close ); break;
        case WM_INITDIALOG: result = HANDLE_WM( INITDIALOG, on_wm::initdialog ); break;
        case WM_PAINT:      result = HANDLE_WM( PAINT,      on_wm::paint ); break;
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
