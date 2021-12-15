#include <windows.h>
#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

using C_str = const char*;
const HINSTANCE this_executable = GetModuleHandle( nullptr );

void set_icon( const HWND window )
{
    const C_str icon_resource_id = MAKEINTRESOURCE( IDI_APP );
    SendMessage( window,
        WM_SETICON, ICON_SMALL,
        (LPARAM) LoadImage( this_executable, icon_resource_id, IMAGE_ICON, 16, 16, {} )
        );
    SendMessage( window,
        WM_SETICON, ICON_BIG,
        (LPARAM) LoadImage( this_executable, icon_resource_id, IMAGE_ICON, 32, 32, {} )
        );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( this_executable, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
}

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_initdialog( const HWND window )
    -> bool
{
    set_icon( window );
    set_rules_text( window );
    return true;    // `true` sets focus to the control specified by the `w_param`.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    /*w_param*/,
    const LPARAM    /*ell_param*/
    ) -> INT_PTR
{
    switch( msg_id ) {
        case WM_CLOSE:          on_close( window ); return true;
        case WM_INITDIALOG:     return on_initdialog( window );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    const C_str dialog_resource_id = MAKEINTRESOURCE( IDD_MAIN_WINDOW );
    DialogBox( this_executable, dialog_resource_id, HWND(), &message_handler );
}
