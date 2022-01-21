#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>     // hopefully, fail

#include <ole2.h>       // OleInitialize, OleUninitialize

#include    <assert.h>

namespace winapi::ole2 {
    namespace cu = cpp::util;
    using cu::hopefully, cu::fail;

    struct Ole_library_usage
    {
        ~Ole_library_usage()
        {
            OleUninitialize();
        }
        
        Ole_library_usage()
        {
            const HRESULT hr = OleInitialize( {} );
            hopefully( SUCCEEDED( hr ) )
                or CPPUTIL_FAIL( "OleInitialize failed" );
        }
    };
}  // namespace winapi::ole2
