#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>
#include <winapi/gdi/Object_.hpp>               // winapi::gdi::(Brush, Pen, Bitmap)

namespace winapi::gdi {
    class Dc::Selection: No_copying
    {
        const Dc&   m_dc;
        HGDIOBJ     m_original_object;
        
    public:
        ~Selection() { SelectObject( m_dc, m_original_object ); }

        template< class Handle >    // Easy to use interface.
        Selection( const Dc& dc, const Object_<Handle>& object ):
            Selection( From_api_handle(), object.handle(), dc )
        {}

        struct From_api_handle {};  // Intentionally explicit, verbose & ugly interface.
        Selection( From_api_handle, const HGDIOBJ object_handle, const Dc& dc ):
            m_dc( dc ),  m_original_object( SelectObject( dc, object_handle ) )
        {}

        auto dc() const -> const Dc& { return m_dc; }

        operator const Dc&() const { return dc(); }     // Supports `+` operator.
        operator HDC() const { return m_dc.handle(); }  // Supports using Selection as DC.
    };

    inline auto operator+( const Dc& dc, const Brush& object )
        -> Dc::Selection
    { return { Dc::Selection::From_api_handle(), object.handle(), dc }; }

    inline auto operator+( const Dc& dc, const Pen& object )
        -> Dc::Selection
    { return { Dc::Selection::From_api_handle(), object.handle(), dc }; }
}  // namespace winapi::gdi
