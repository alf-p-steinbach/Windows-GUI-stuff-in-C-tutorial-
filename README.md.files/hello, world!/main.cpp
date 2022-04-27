#include <windows.h>
#include <commctrl.h>           // InitCommonControlsEx, ICC_*
#include "resource-ids.h"       // IDI_MAIN

#ifdef _MSC_VER
#   pragma comment( lib, "Comctl32" )
#endif

constexpr auto utf8_is_the_execution_character_set()
    -> bool
{
    constexpr auto& slashed_o = "ø";
    return (sizeof( slashed_o ) == 3 and slashed_o[0] == '\xC3' and slashed_o[1] == '\xB8');
}

static_assert(
    utf8_is_the_execution_character_set(),
    "The execution character set must be UTF-8 (e.g. MSVC option \"/utf-8\")."
    );


constexpr DWORD basic_common_controls = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;

auto init_common_controls( const DWORD which = basic_common_controls )
    -> bool
{
    const INITCOMMONCONTROLSEX params = {sizeof( params ), which};
    return !!InitCommonControlsEx( &params );
}

auto main()
    -> int
{
    if( GetACP() != CP_UTF8 ) {
        return EXIT_FAILURE;    // Not UTF-8 support for MessageBox.
    }

    init_common_controls();

    MSGBOXPARAMS params = {sizeof( MSGBOXPARAMS )};
    params.lpszCaption  = "Hello, world!";
    params.lpszText     = "Did you know, that almost every 日本国 кошка likes Norwegian blåbærsyltetøy?\n"
                          "\n"
                          "(Click the OK button to quit, please)";
    params.lpszIcon     = MAKEINTRESOURCE( IDI_MAIN );
    params.hInstance    = GetModuleHandle( nullptr );
    params.dwStyle      = MB_SETFOREGROUND | MB_USERICON | MB_TASKMODAL;    // Windows 11.
    
    MessageBoxIndirect( &params );
}
