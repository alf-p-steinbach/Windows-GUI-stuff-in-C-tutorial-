// v1 - Missing window parts added programmatically: the rules text; the window icon.
// v0 - Roughly minimum code to display a window based on a dialog template resource.

#include <windows.h>
#undef small    // MSVC <windows.h> includes <rpcndr.h> which defines `small` as macro. :(

#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW, IDI_APP


//------------------------------------------- Support machinery:

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
    LoadString( this_exe , IDS_RULES, text, sizeof( text ) );
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
    set_app_icon( window );
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
    const C_str id_as_ptr = MAKEINTRESOURCE( IDD_MAIN_WINDOW );
    DialogBox( this_exe, id_as_ptr, HWND(), message_handler );
}
