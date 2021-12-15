// v0 - Roughly minimum code to display a window based on a dialog template resource.

#include <windows.h>
#include "resources.h"

using C_str = const char*;

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    /*w_param*/,
    const LPARAM    /*ell_param*/
    ) -> INT_PTR
{
    switch( msg_id ) {
        case WM_CLOSE: {
            EndDialog( window, IDOK );      // Without this the window won't close.
            return true;
        }
    }
    return false;       // Didn't process the message, want default.
}

auto main() -> int
{
    const HINSTANCE this_executable     = GetModuleHandle( nullptr );
    const C_str     resource_id_as_ptr  = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    DialogBox( this_executable, resource_id_as_ptr, HWND(), &message_handler );
}
