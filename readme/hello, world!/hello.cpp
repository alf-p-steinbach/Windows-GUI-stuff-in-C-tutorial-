#include <windows.h>

auto main() -> int
{
    MSGBOXPARAMS params = {sizeof( MSGBOXPARAMS )};
    
    params.lpszCaption  = "Hello, world!";
    params.lpszText     = "Click the OK button to quit, please:";

    params.lpszIcon     = MAKEINTRESOURCE( 100 );
    params.hInstance    = GetModuleHandle( nullptr );
    
    params.dwStyle      = MB_SETFOREGROUND | MB_USERICON;
    
    MessageBoxIndirect( &params );
}
