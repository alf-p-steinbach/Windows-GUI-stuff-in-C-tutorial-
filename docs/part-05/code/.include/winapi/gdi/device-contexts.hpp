#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::(hopefully, fail)
#include <winapi/gdi/Object_.hpp>           // winapi::gdi::Bitmap
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying, cu::Std_ref_;

    class Dc: No_copying
    {
        HDC     m_handle;
       
    protected:
        inline virtual ~Dc() = 0;       // Derived-class responsibility.

        Dc( const HDC handle ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
        }

    public:    
        auto handle() const -> HDC { return m_handle; }
    };

    inline Dc::~Dc() {}


    class Screen_dc: public Dc
    {
    public:        
        ~Screen_dc() override { ReleaseDC( 0, handle() ); }

        Screen_dc():
            Dc( GetDC( 0 ) )                // Main screen specified implicitly.
        {}
    };


    class Memory_dc: public Dc
    {
    public:
        ~Memory_dc() override { DeleteDC( handle() ); }

        Memory_dc():
            Dc( CreateCompatibleDC( 0 ) )   // Screen DC for main screen specified implicitly.
        {}
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
}  // namespace winapi::gdi
