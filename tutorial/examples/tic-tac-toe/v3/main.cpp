// v3 - Refactoring: <windows.h> wrapped; using <windowsx.h> macros; resource-id class.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimum code to display a window based on a dialog template resource.

#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    
#include <windowsx.h>               // HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW


//------------------------------------------- Support machinery:

// Invokes various <windowsx.h> “message cracker” macros like `HANDLE_WM_CLOSE`. Each such
// macro interprets WPARAM and LPARAM depending on the message id, and in turn invokes a
// specified handler func with message dependent arguments created from WPARAM and LPARAM.
#define HANDLER_OF( msg_name, handler_func ) \
    HANDLE_##msg_name( msg.hwnd, msg.wParam, msg.lParam, handler_func )

const HINSTANCE this_exe = GetModuleHandle( nullptr );

using C_str = const char*;
struct Icon_kind{ enum Enum{ small = ICON_SMALL, big = ICON_BIG }; };

struct Resource_id
{
    int value;
    auto as_ptr() const -> C_str { return MAKEINTRESOURCE( value ); }
};

void set_icon( const HWND window, const Icon_kind::Enum kind, const Resource_id id )
{
    const int       size    = (kind == Icon_kind::small? 16 : 32);
    const HANDLE    icon    = LoadImage( this_exe, id.as_ptr(), IMAGE_ICON, size, size, {} );
    SendMessage( window, WM_SETICON, kind, reinterpret_cast<LPARAM>( icon ) );
}


//------------------------------------------- App code:

void set_app_icon( const HWND window )
{
    set_icon( window, Icon_kind::small, Resource_id{ IDI_APP } );
    set_icon( window, Icon_kind::big, Resource_id{ IDI_APP } );
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
    const MSG msg = {window, msg_id, w_param, ell_param};   // Used by HANDLER_OF.
    switch( msg_id ) {
        case WM_CLOSE:          return HANDLER_OF( WM_CLOSE, on_close );
        case WM_INITDIALOG:     return HANDLER_OF( WM_INITDIALOG, on_initdialog );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, Resource_id{ IDD_MAIN_WINDOW }.as_ptr(), HWND(), message_handler );
}
