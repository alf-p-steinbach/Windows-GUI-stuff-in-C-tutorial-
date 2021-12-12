#include <windows.h>
#include "resources.h"

const HINSTANCE this_exe = GetModuleHandle( nullptr );

auto message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    (void) w_param; (void) ell_param;
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
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
