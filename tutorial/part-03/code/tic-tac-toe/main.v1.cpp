#include <windows.h>
#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

const HINSTANCE this_exe = GetModuleHandle( nullptr );

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_initdialog( const HWND window )
    -> bool
{
    char text[2048];
    LoadString( this_exe, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
    return true;    // `true` sets focus to the control specified by the `w_param`.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    (void) w_param; (void) ell_param;

    switch( msg_id ) {
        case WM_CLOSE:          on_close( window ); return true;
        case WM_INITDIALOG:     return on_initdialog( window );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
