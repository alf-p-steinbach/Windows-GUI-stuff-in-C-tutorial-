#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi-headers/windows-h.hpp>
#include <cpp/util.hpp>

#include    <assert.h>
#include    <utility>       // std::(enable_if_t, exchange)

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::No_copying, cu::Types_;
    using   std::enable_if_t, std::exchange;

    // The handle types below are the types that have destruction via GDI `DestroyObject`.
    // The `HGDIOBJ` handle type is like a base class for the other handle types.
    // Note: the GDI `GetObjectType` function supports some additional GDI object kinds.
    using Object_handle_types = Types_<HGDIOBJ, HPEN, HBRUSH, HFONT, HBITMAP, HRGN, HPALETTE>;

    template<
        class Handle_type,
        class = enable_if_t< Object_handle_types::contain_< Handle_type > >
        >
    class Object_: No_copying
    {
    public:
        using Handle = Handle_type;

    private:
        Handle      m_handle;
        
    public:
        ~Object_()
        {
            if( m_handle ) {
                // Deletion may possibly fail if the object is selected in a device context.
                const bool deleted = ::DeleteObject( m_handle );
                assert(( "DeleteObject", deleted ));  (void) deleted;
            }
        }
        
        Object_( Handle&& handle ): m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Handle is 0." );
        }

        auto released() -> Handle { return exchange( m_handle, Handle( 0 ) ); }

        auto handle() const -> Handle { return m_handle; }
        operator Handle() const { return handle(); }
    };

    using   Object      = Object_<HGDIOBJ>;

    using   Pen         = Object_<HPEN>;
    using   Brush       = Object_<HBRUSH>;
    using   Font        = Object_<HFONT>;
    using   Bitmap      = Object_<HBITMAP>;
    using   Region      = Object_<HRGN>;
    using   Palette     = Object_<HPALETTE>;
}  // namespace winapi::gdi
