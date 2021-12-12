#include <windows.h>
#include <windowsx.h>       // HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

// Invokes various <windowsx.h> macros that in turn invoke specified message handler funcs:
#define CALL_HANDLER_OF( msg_name, args, func )  HANDLE_##msg_name( args, func )

const HINSTANCE this_exe = GetModuleHandle( nullptr );

// A Windows 11 workaround hack. The window is assumed to presently be a “topmost” window.
// The effect is to bring the window to the top of ordinary window z-order.
void remove_topmost_style_for( const HWND window )
{
    SetWindowPos( window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

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

    remove_topmost_style_for( window );

    return true;    // `true` sets focus to the control specified by the `focus` param.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    #define PARAMS     window, w_param, ell_param
    switch( msg_id ) {
        case WM_CLOSE:          return CALL_HANDLER_OF( WM_CLOSE, PARAMS, on_close );
        case WM_INITDIALOG:     return CALL_HANDLER_OF( WM_INITDIALOG, PARAMS, on_initdialog );
    }
    #undef PARAMS
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
