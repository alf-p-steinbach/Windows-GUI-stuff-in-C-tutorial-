// v3-1 - Refactoring: <windows.h> wrapped.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW, ...
#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    

#include <stdlib.h>                 // EXIT_...

using Char_ptr = const char*;


//------------------------------------------- API support machinery:

const HINSTANCE this_executable = GetModuleHandle( nullptr );

namespace icon_sizes{
    enum Enum{ small = ICON_SMALL, large = ICON_BIG };       // WM_SETICON values.
}  // namespace icon_sizes

void set_icon( const HWND window, const icon_sizes::Enum size, const int resource_id )
{
    const Char_ptr  id_as_pseudo_ptr    = MAKEINTRESOURCE( resource_id );
    const int       pixel_size          = (size == icon_sizes::small? 16 : 32);
    const HANDLE    icon                = LoadImage(
        this_executable, id_as_pseudo_ptr, IMAGE_ICON, pixel_size, pixel_size, {}
        );
    SendMessage( window, WM_SETICON, size, reinterpret_cast<LPARAM>( icon ) );
}


//------------------------------------------- App code:

void set_app_icon( const HWND window )
{
    set_icon( window, icon_sizes::small, IDI_APP );
    set_icon( window, icon_sizes::large, IDI_APP );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( this_executable, IDS_RULES, text, sizeof( text ) );
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

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    ,       // w_param
    const LPARAM            // ell_param
    ) -> INT_PTR
{
    switch( msg_id ) {
        case WM_CLOSE:          on_wm_close( window ); return true;
        case WM_INITDIALOG:     return on_wm_initdialog( window );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    const Char_ptr  resource_id_as_pseudo_ptr   = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    // Note: that there /is/ a return value is undocumented. Can fail if no dialog resource.
    const auto return_value = DialogBox(
        this_executable, resource_id_as_pseudo_ptr, HWND( 0 ), message_handler
        );
    return (return_value <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
