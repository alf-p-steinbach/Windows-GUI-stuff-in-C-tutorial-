// v2 - Refactoring of v1, mostly by re-expressing it with <windowsx.h> macros.
// v1 - Code to add the missing window things: the game's rule text; the window icon.
// v0 - Roughly minimum code to display a window based on a dialog template resource.

#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    
#include <windowsx.h>               // HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

//------------------------------------------- Support machinery:

// Invokes various <windowsx.h> macros that in turn invoke specified message handler funcs:
#define CALL_HANDLER_OF( msg_name, handler_func ) \
    HANDLE_##msg_name( ch_params.hwnd, ch_params.wParam, ch_params.lParam, handler_func )

const HINSTANCE this_exe = GetModuleHandle( nullptr );

using C_str = const char*;
struct Icon_kind{ enum Enum{ small = ICON_SMALL, big = ICON_BIG }; };

void set_icon( const HWND window, const Icon_kind::Enum kind, const int resource_id )
{
    const C_str     id_as_ptr   = MAKEINTRESOURCE( resource_id );
    const int       size        = (kind == Icon_kind::small? 16 : 32);
    const HANDLE    icon        = LoadImage( this_exe, id_as_ptr, IMAGE_ICON, size, size, {} );
    SendMessage( window, WM_SETICON, kind, reinterpret_cast<LPARAM>( icon ) );
}


//------------------------------------------- App code:

void set_app_icon( const HWND window )
{
    set_icon( window, Icon_kind::small, IDI_APP );
    set_icon( window, Icon_kind::big, IDI_APP );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( this_exe, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
}

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_initdialog( const HWND window, const HWND /*focus*/, const LPARAM /*ell_param*/ )
    -> bool
{
    set_app_icon( window );
    set_rules_text( window );
    return true;    // `true` sets focus to the `focus` control.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG ch_params = {window, msg_id, w_param, ell_param}; // Used by CALL_HANDLER_OF.
    switch( msg_id ) {
        case WM_CLOSE:          return CALL_HANDLER_OF( WM_CLOSE, &on_close );
        case WM_INITDIALOG:     return CALL_HANDLER_OF( WM_INITDIALOG, on_initdialog );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
