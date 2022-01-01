#ifndef UNICODE
#   error "Gah, no UNICODE"
#endif
#include <Windows.h>

#include "resources.h"

template< class T > using T_ = T;

auto main() -> int
{
    const HINSTANCE this_exe = GetModuleHandle( nullptr );
    const wchar_t* p_hello_text;
    LoadString( this_exe, IDS_HELLO, reinterpret_cast<wchar_t*>( &p_hello_text ), 0 );
    MessageBox( 0, p_hello_text, L"UTF-16 from UTF-8 resource spec:", MB_SETFOREGROUND );
}
