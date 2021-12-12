#include <windows.h>
#include "resources.h"

const HINSTANCE this_exe = GetModuleHandle( nullptr );

INT_PTR event_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    )
{
    (void) w_param; (void) ell_param;
    switch( msg_id ) {
        case WM_CLOSE: {
            EndDialog( window, EXIT_SUCCESS );
            return true;
        }
        case WM_INITDIALOG: {
            char text[2048];
            LoadString( this_exe, IDS_RULES, text, sizeof( text ) );
            const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
            SetWindowText( rules_display, text );
            return true;
        }
    }
    return false;       // Didn't process the message, want default.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &event_handler );
}
