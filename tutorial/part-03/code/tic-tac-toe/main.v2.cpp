#include <windows.h>
#include <windowsx.h>       // HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

// Invokes various <windowsx.h> macros that in turn invoke specified message handler funcs:
#define CALL_HANDLER( msg_name, handler_func ) \
    HANDLE_##msg_name( ch_params.hwnd, ch_params.wParam, ch_params.lParam, handler_func )

const HINSTANCE this_exe = GetModuleHandle( nullptr );

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_initdialog( const HWND window, const HWND focus, const LPARAM ell_param )
    -> bool
{
    (void) focus;  (void) ell_param;
    
    char text[2048];
    LoadString( this_exe, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
    return true;    // `true` sets focus to the control specified by the `focus` param.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG ch_params = {window, msg_id, w_param, ell_param};
    switch( msg_id ) {
        case WM_CLOSE:          return CALL_HANDLER( WM_CLOSE, &on_close );
        case WM_INITDIALOG:     return CALL_HANDLER( WM_INITDIALOG, on_initdialog );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
