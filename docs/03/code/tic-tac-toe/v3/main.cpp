// v3 - Refactoring: <windows.h> wrapped; using <windowsx.h> macros; resource-id class.
//      "winapi-support-machinery.hpp" introduced as place for Windows API support stuff.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimum code to display a window based on a dialog template resource.

#include "winapi_support_machinery.hpp" // winapi_support_machinery::*, WSM_HANDLE_WM
#include "resources.h"                  // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW, ...

#include <optional>         // std::optional
#include <tuple>            // std::ignore

namespace wsm = winapi_support_machinery;
using   std::optional,
        std::ignore;

using Char_ptr = const char*;

void set_app_icon( const HWND window )
{
    wsm::set_icon( window, wsm::Resource_id{ IDI_APP } );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( wsm::this_executable, IDS_RULES, text, sizeof( text ) );
    SetDlgItemText( window, IDC_RULES_DISPLAY, text );
}

namespace on_wm {
    void close( const HWND window )
    {
        EndDialog( window, IDOK );
    }

    auto initdialog( const HWND window, const HWND focus, const LPARAM ell_param )
        -> bool
    {
        ignore = focus;  ignore = ell_param;

        set_app_icon( window );
        set_rules_text( window );
        return true;    // `true` sets focus to the `focus` control.
    }
}  // namespace on_wm

auto handling_of( const MSG& msg )
    -> optional<INT_PTR>
{
    switch( msg.message ) {
        case WM_CLOSE:          return WSM_HANDLE_WM( CLOSE, on_wm::close );
        case WM_INITDIALOG:     return WSM_HANDLE_WM( INITDIALOG, on_wm::initdialog );
    }
    return {};
}

auto CALLBACK message_handler_callback(
    const HWND          window,
    const UINT          msg_id,
    const WPARAM        w_param,
    const LPARAM        ell_param
    ) -> INT_PTR
{
    // Optional has value is `false` => Didn't process the message, want default processing.
    const optional<INT_PTR> result = handling_of({ window, msg_id, w_param, ell_param });
    return (result.has_value()? SetDlgMsgResult( window, msg_id, result.value() ) : false);
}

auto main() -> int
{
    // Note: that there /is/ a return value is undocumented. Can fail if no dialog resource.
    const auto return_value = DialogBox(
        wsm::this_executable,
        wsm::Resource_id{ IDD_MAIN_WINDOW }.as_pseudo_ptr(),
        HWND( 0 ),
        message_handler_callback
        );
    return (return_value <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
