#pragma once // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>     // CPPUTIL_FAIL, cpp::util::(hopefully, int_size)

#include <string>
#include <string_view>

namespace winapi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::fail, cu::int_size;
    using   std::wstring,
            std::string_view;
    
    inline auto to_utf16( const string_view& s )
        -> wstring
    {
        const auto s_length = int_size( s );
        if( s_length == 0 ) { return L""; }

        const auto buffer_size = s_length;  // May be a litte too large, but that's OK.
        auto result = wstring( buffer_size, L'\0' );
        constexpr auto flags = DWORD( 0 );
        const int n_wide_values = MultiByteToWideChar(
            CP_UTF8, flags, s.data(), s_length, &result[0], buffer_size
            );
        hopefully( n_wide_values != 0 ) or CPPUTIL_FAIL( "MultiByteToWideChar failed" );
        result.resize( n_wide_values );
        return result;
    }
}  // namespace winapi
