// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include <windows.h>
#include "resources.h"
using Char_ptr = const char*;

auto CALLBACK message_handler(
    const HWND              window,
    const UINT              msg_id,
    const WPARAM            ,       // w_param
    const LPARAM                    // ell_param
    ) -> INT_PTR
{
    if( msg_id == WM_CLOSE ) {
        EndDialog( window, IDOK );  // Without this the window won't close.
        return true;
    }
    return false;       // Didn't process the message, want default.
}

auto main() -> int
{
    const HINSTANCE this_executable             = GetModuleHandle( nullptr );
    const Char_ptr  resource_id_as_pseudo_ptr   = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    DialogBox( this_executable, resource_id_as_pseudo_ptr, HWND(), message_handler );
}
