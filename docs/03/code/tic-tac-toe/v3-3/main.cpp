// v3-3 - Refactoring: reusable support machinery in separate headers.
// v3-2 - Support for general message processing results in dialog proc.
// v3-1 - Refactoring: <windows.h> wrapped.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include "resources.h"                  // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW, ...
#include <cpp_support_machinery.hpp>    // cpp_support_machinery::*
#include <winapi_support_machinery.hpp> // winapi_support_machinery::*

#include <stdlib.h>                 // EXIT_...

#include <iterator>                 // std::(begin, end)
#include <optional>                 // std::optional

namespace csm   = cpp_support_machinery;
namespace wsm   = winapi_support_machinery;

using   csm::contains_;
using   std::optional;

void set_app_icon( const HWND window )
{
    wsm::icon::set_in( window, wsm::resource::Id( IDI_APP ) );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    { const auto spec = wsm::resource::Location( IDS_RULES );
        LoadString( spec.module(), spec.id(), text, sizeof( text ) );
    }
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
}

void on_wm_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_wm_initdialog( const HWND window )
    -> bool
{
    set_app_icon( window );
    set_rules_text( window );
    return true;    // `true` sets focus to the control specified by the `w_param`.
}

auto message_handler( const MSG& msg )
    -> optional<INT_PTR>
{
    using wsm::dialog_message::processed;
    switch( msg.message ) {
        case WM_CLOSE:          on_wm_close( msg.hwnd ); return processed;
        case WM_INITDIALOG:     return on_wm_initdialog( msg.hwnd );
    }
    return {};
}

auto main() -> int
{
    // Note: that there /is/ a return value is undocumented. Can fail if no dialog resource.
    const auto spec = wsm::resource::Location( IDD_MAIN_WINDOW );
    const auto return_value = DialogBox(
        spec.module(), spec.id().as_pseudo_ptr(),
        HWND( 0 ),
        &wsm::dialog_message::callback_<message_handler>
        );
    return (return_value <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
