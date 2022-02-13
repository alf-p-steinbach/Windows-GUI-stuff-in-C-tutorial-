#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::Success
#include <wrapped-winapi/windows-h.hpp>     // HRESULT, SetLastError, GetLastError

namespace winapi::com {
    inline namespace failure_checking {
        using cpp::util::Success;

        constexpr auto denotes_success( const HRESULT hr )
            -> bool
        { return SUCCEEDED( hr ); }         // Essentially that hr >= 0.

        inline auto operator>>( const HRESULT hr, Success )
            -> bool
        {
            const bool is_success = denotes_success( hr );
            if( not is_success ) { SetLastError( hr ); }    // Can be retrieved via GetLastError.
            return is_success;
        }
    }  // namespace failure_checking
}  // namespace winapi::com
