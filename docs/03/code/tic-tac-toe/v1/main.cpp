// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include <windows.h>
#include <stdlib.h>         // EXIT_...
#include "resources.h"
using Char_ptr = const char*;

auto CALLBACK message_handler(
    const HWND              window,
    const UINT              msg_id,
    const WPARAM            ,           // w_param
    const LPARAM                        // ell_param
    ) -> INT_PTR    
{   
    if( msg_id == WM_CLOSE ) {  
        EndDialog( window, IDOK );      // Without this the window won't close.
        return true;
    }
    return false;   // Didn't process the message, want default.
}

auto main() -> int
{
    const HINSTANCE this_executable             = GetModuleHandle( nullptr );
    const Char_ptr  resource_id_as_pseudo_ptr   = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    // Note: that there /is/ a return value is undocumented. Can fail if no dialog resource.
    const auto return_value = DialogBox(
        this_executable, resource_id_as_pseudo_ptr, HWND( 0 ), message_handler
        );
    return (return_value <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
