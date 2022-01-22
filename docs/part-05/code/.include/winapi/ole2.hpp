#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>     // CPPUTIL_FAIL, cpp::util::hopefully

#include <ole2.h>   // OleInitialize, OleUninitialize

namespace winapi::ole2 {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying;

    struct Library_usage: No_copying
    {
        Library_usage()
        {
            const HRESULT hr = OleInitialize( {} );
            hopefully( SUCCEEDED( hr ) ) or CPPUTIL_FAIL( "OleInitialize failed" );
        }

        ~Library_usage()
        {
            OleUninitialize();
        }
    };
}  // namespace winapi::ole2
