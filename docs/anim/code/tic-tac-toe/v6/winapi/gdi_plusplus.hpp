#pragma once
#include <wrapped-winapi/gdiplus-h.hpp>
#include <cpp/util.hpp>

namespace winapi::gdi_plusplus {
    namespace api   = Gdiplus;
    namespace cu    = cpp::util;
    using cu::hopefully, cu::No_copying;
    
    class Library:
        private No_copying
    {
        ULONG_PTR           m_token;

    public:
        ~Library() { api::GdiplusShutdown( m_token ); }

        Library()
        {
            auto params = api::GdiplusStartupInput();
            const api::Status result = api::GdiplusStartup( &m_token, &params, nullptr );
            hopefully( result == api::Ok )
                or CPPUTIL_FAIL( "GdiplusStartup failed" );
        }
    };
}  // namespace winapi::gdi_plusplus
