#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                             // cpp::util::(hopefully, fail)
#include <winapi/kernel/encoding-conversions.hpp>   // winapi::kernel::to_utf16
#include <wrapped-winapi-headers/oleauto-h.hpp>             // BSTR, SysAllocStringLen, SysFreeString

#include <string_view>      // std::(string_view, wstring_view)

namespace winapi::ole {
    namespace cu        = cpp::util;
    namespace kernel    = winapi::kernel;

    using cu::int_size, cu::hopefully, cu::No_copying;
    using kernel::to_utf16;
    using std::string_view, std::wstring_view;

    class B_string: No_copying
    {
        BSTR    m_pointer;
        
    public:
        ~B_string() { SysFreeString( m_pointer ); }

        B_string( const wstring_view& ws ):
            m_pointer( SysAllocStringLen( ws.data(), 1u*int_size( ws ) ) )
        {
            hopefully( m_pointer ) or CPPUTIL_FAIL( "SysAllocStringLen failed" );
        }
        
        B_string( const string_view& s ): B_string( to_utf16( s ) ) {}
        
        operator BSTR() const { return m_pointer; }     // Intentionally ignores C++20 `<=>`.
    };
} // namespace winapi::ole
