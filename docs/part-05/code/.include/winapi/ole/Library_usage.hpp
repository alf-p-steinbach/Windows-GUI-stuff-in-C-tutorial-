#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                 // CPPUTIL_FAIL, cpp::util::hopefully
#include <wrapped-winapi/ole2-h.hpp>    // OleInitialize, OleUninitialize

namespace winapi::ole {
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
}  // namespace winapi::ole
