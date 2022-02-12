#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::(hopefully, fail)
#include <winapi/gdi/Object_.hpp>           // winapi::gdi::(Brush, Pen, Bitmap)
#include <winapi/gui/std_font.hpp>          // winapi::gui::std_font
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying, cu::Std_ref_;

    inline void make_practical( const HDC dc )
    {
        SelectObject( dc, GetStockObject( DC_PEN ) );
        SelectObject( dc, GetStockObject( DC_BRUSH ) );
        SetBkMode( dc, TRANSPARENT );       // Don't fill in the background of text, please.
        SelectObject( dc, gui::std_font );
    }

    class Dc: No_copying
    {
        HDC     m_handle;
       
    protected:
        inline virtual ~Dc() = 0;                           // Derived-class responsibility.

        Dc( HDC&& handle, const bool do_extended_init = true ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
            if( do_extended_init ) { make_practical( m_handle ); }
        }

    public:
        class Selection;                                    // RAII for SelectObject, separate.

        auto handle() const -> HDC { return m_handle; }
        operator HDC() const { return handle(); }
    };

    inline Dc::~Dc() {}


    class Screen_dc: public Dc
    {
    public:        
        ~Screen_dc() override { ReleaseDC( 0, handle() ); }
        Screen_dc(): Dc( GetDC( 0 ) ) {}                    // Main screen specified implicitly.
    };


    class Memory_dc: public Dc
    {
    public:
        ~Memory_dc() override { DeleteDC( handle() ); }
        Memory_dc(): Dc( CreateCompatibleDC( 0 ) ) {}       // Implicitly DC for main screen.
    };


    class Bitmap_dc: public Memory_dc
    {
        Bitmap* m_p_bitmap;

    public:
        Bitmap_dc( const Std_ref_<Bitmap> bitmap ):
            Memory_dc(),
            m_p_bitmap( &bitmap.get() )
        {
            SelectObject( handle(), m_p_bitmap->handle() );
        }
        
        auto bitmap() const -> const Bitmap& { return *m_p_bitmap; }
    };


    class Dc::Selection: No_copying
    {
        const Dc&   m_dc;
        HGDIOBJ     m_original_object;
        
    public:
        ~Selection() { SelectObject( m_dc, m_original_object ); }

        struct From_api_handle {};  // Intentionally explicit, verbose & ugly interface.
        Selection( From_api_handle, const HGDIOBJ object_handle, const Dc& dc ):
            m_dc( dc ),  m_original_object( SelectObject( dc, object_handle ) )
        {}

        template< class Handle >    // Easy to use interface.
        Selection( const Dc& dc, const Object_<Handle>& object ):
            Selection( From_api_handle(), object.handle(), dc )
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
