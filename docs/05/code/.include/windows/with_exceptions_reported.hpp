#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include "wrapped-winapi-headers/windows-h.hpp"

#include <stdio.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace windows {
    using   std::function, std::exception, std::string, std::string_view;

    inline void error_box( const string& title, string const& text )
    {
        const auto errorbox_flags = MB_ICONERROR | MB_SYSTEMMODAL;  // Sys-modal for Win 11.
        MessageBox( 0, text.c_str(), title.c_str(), errorbox_flags );
    }

    inline auto with_exceptions_reported(
        function<void()>        cpp_main_func,
        const string_view&      app_name     = "This program"
        ) -> int
    {
        try {
            cpp_main_func();
            return EXIT_SUCCESS;
        } catch( const exception& x ) {
            fflush( stdout );
            fprintf( stderr, "!%s\n", x.what() );
            fflush( stderr );
            const auto s = string();
            error_box( string( app_name ) + " failed:", s + "Because:\n" + x.what() );
        }
        return EXIT_FAILURE;
    }
}  // namespace windows
