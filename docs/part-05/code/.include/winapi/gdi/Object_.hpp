#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>

#include    <assert.h>
#include    <utility>       // std::(enable_if_t, exchange)

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::No_copying, cu::includes_type_;
    using   std::is_same_v,
            std::enable_if_t, std::exchange;

    template<
        class Handle,
        class = enable_if_t< includes_type_< Handle,
            HGDIOBJ, HPEN, HBRUSH, HFONT, HBITMAP, HRGN, HPALETTE
            > >
        >
    class Object_: No_copying
    {
        Handle      m_handle;
        
    public:
        ~Object_()
        {
            const bool ok = !!::DeleteObject( m_handle );
            assert(( "DeleteObject", ok ));  (void) ok;
        }
        
        Object_( const Handle handle ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Handle is 0." );
        }
        
        auto released() -> Handle { return exchange( m_handle, Handle( 0 ) ); }

        auto handle() const -> Handle { return m_handle; }
        operator Handle() const { return handle(); }
    };
}  // namespace winapi::gdi
