#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::(hopefully, fail)
#include <wrapped-winapi/ole2-h.hpp>

#include <string_view>

namespace winapi::ole {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying;
    using std::string_view, std::wstring_view;

    class B_string: No_copying
    {
        BSTR    m_pointer;
        
    public:
        ~B_string() { SysFreeString( m_pointer ); }

        B_string( const wstring_view& ws ):
            m_pointer( SysAllocStringLen( ws.data(), int_size( ws ) ) )
        {
            hopefully( m_pointer ) or CPPUTIL_FAIL( "SysAllocStringLen failed" );
        }
        
        B_string( const string_view& s ): B_string( winapi::to_utf16( s ) ) {}
        
        operator BSTR() const { return m_pointer; }     // Intentionally ignores C++20 `<=>`.
    };
} // namespace winapi::ole
