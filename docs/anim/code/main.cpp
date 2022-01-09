#// Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.

#include <winapi/util.hpp>          // winapi_util::*
#include <cpp/util.hpp>
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

#include <assert.h>
#include <stdlib.h>     // EXIT_...

#include <string>

namespace cu    = cpp::util;
namespace wu    = winapi::util;

static_assert(
    cu::utf8_is_the_execution_character_set(),
    "The execution character set must be UTF-8 (e.g. MSVC option \"/utf-8\")."
    );

using   cu::hopefully, cu::Range, cu::is_in;
using   std::exception, std::optional, std::string, std::to_string;
using   ttt::Board, ttt::Game;
#define FAIL CPPUTIL_FAIL

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    optional<INT_PTR> result;

    #define HANDLE_WM( name, handler_func ) \
        HANDLE_WM_##name( window, w_param, ell_param, handler_func )
    switch( msg_id ) {
    }
    #undef HANDLE_WM

    // `false` => Didn't process the message, want default processing.
    return (result? SetDlgMsgResult( window, msg_id, result.value() ) : false);
}

void cpp_main()
{
    wu::init_common_controls();
    DialogBox(
        wu::this_exe, wu::Resource_id{ IDD_MAIN_WINDOW }.as_ptr(),
        HWND(),             // Parent window, a zero handle is "no parent".
        message_handler
        );
}

auto main() -> int
{
    try {
        static_assert( CP_UTF8 == 65001 );
        hopefully( GetACP() == CP_UTF8 )
            or FAIL( "The process ANSI codepage isn't UTF-8." );
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        const string text = string() +
            "Sorry, there was an unexpected failure.\n"
            "\n"
            "Technical reason (exception message):\n"
            + x.what();
        MessageBox( 0, text.c_str(), "Tic-Tac-Toe - OOPS!", MB_ICONERROR | MB_SETFOREGROUND );
    }
    return EXIT_FAILURE;
}
