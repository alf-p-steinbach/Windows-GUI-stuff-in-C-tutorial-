#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::(hopefully, fail)
#include <winapi/gdi-Bitmap.hpp>            // winapi::gdi::*
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying;

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
        static constexpr auto no_window = HWND( 0 );

    public:        
        ~Screen_dc() override { ReleaseDC( no_window, handle() ); }

        Screen_dc():
            Dc( GetDC( no_window ) )
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
        Bitmap  m_bitmap;

    public:
        Bitmap_dc( const int width, const int height ):
            Memory_dc(),
            m_bitmap( width, height )
        {
            SelectObject( handle(), m_bitmap.handle() );
        }
    };
}  // namespace winapi::gdi
