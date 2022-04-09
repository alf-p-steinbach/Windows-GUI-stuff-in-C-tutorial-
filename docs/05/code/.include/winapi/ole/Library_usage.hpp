#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // CPPUTIL_FAIL, cpp::util::hopefully
#include <winapi/com/failure-checking.hpp>  // winapi::com::failure_checking::(success, >>)
#include <wrapped-winapi-headers/ole2-h.hpp>        // OleInitialize, OleUninitialize

namespace winapi::ole {
    using namespace winapi::com::failure_checking;
    using cpp::util::No_copying;

    struct Library_usage: No_copying
    {
        Library_usage()
        {
            OleInitialize( {} )
                >> success or CPPUTIL_FAIL( "OleInitialize failed" );
        }

        ~Library_usage()
        {
            OleUninitialize();  // Failure handling here would be advanced, e.g. logging.
        }
    };
}  // namespace winapi::ole
