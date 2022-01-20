#pragma once // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>     // hopefully, fail

#include <string>
#include <string_view>

namespace winapi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::fail;
    using   std::string, std::wstring,
            std::string_view, std::wstring_view;
    
    inline auto to_utf16( const string_view& s )
        -> wstring
    {
        if( s.empty() ) { return L""; }

        const auto s_length     = static_cast<int>( s.length() );
        const auto buffer_size  = s_length;
        
        auto result = wstring( buffer_size, L'\0' );
        assert( result.size() == buffer_size );
        constexpr auto flags = DWORD( 0 );
        const int n_wide_values = MultiByteToWideChar(
            CP_UTF8, flags, s.data(), s_length, &result[0], buffer_size
            );
        hopefully( n_wide_values != 0 ) or fail( "MultiByteToWideChar failed" );
        result.resize( n_wide_values );
        return result;
    }

}  // namespace winapi
