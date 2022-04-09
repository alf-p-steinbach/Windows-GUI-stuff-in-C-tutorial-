#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                         // cpp::util::(hopefully, No_copying, Types_)
#include <winapi/gdi/color-usage-classes.hpp>   // winapi::gdi::(Brush_color, Pen_color, Gap_color)
#include <winapi/gui/std_font.hpp>              // winapi::gui::std_font
#include <wrapped-winapi-headers/windows-h.hpp>         // General Windows API.

#include <stddef.h>         // size_t

#include <string>           // std::string
#include <string_view>      // std::string_view

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::No_copying, cu::Types_;
    using   std::string,
            std::string_view,
            std::get, std::tie, std::tuple,
            std::is_same_v,
            std::index_sequence, std::make_index_sequence;

    inline void make_practical( const HDC dc )
    {
        SelectObject( dc, GetStockObject( DC_PEN ) );
        SelectObject( dc, GetStockObject( DC_BRUSH ) );
        SetBkMode( dc, TRANSPARENT );   // Don't fill in background of text, please.
        SelectObject( dc, gui::std_font );
    }

    class Dc: No_copying
    {
        const HDC   m_handle;

    protected:
        struct No_extended_init {};

        inline virtual ~Dc() = 0;                           // Derived-class responsibility.

        Dc( HDC&& handle, No_extended_init ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 )
                or CPPUTIL_FAIL( "Device context handle is 0." );
        }

        Dc( HDC&& handle ):
            Dc( move( handle ), No_extended_init() )
        {
            make_practical( m_handle );
        }

    public:
        template< class... Args >
        inline auto use( const Args&... colors ) -> Dc&;
     
        // Convenience special cases.
        auto bg( const Brush_color color )  -> Dc&  { return use( color ); }
        auto fg( const Pen_color color )    -> Dc&  { return use( color ); }
        
        inline auto fill( const RECT& area ) -> Dc&;
        
        template< class Api_func, class... Args >
        inline auto draw( const Api_func f, const Args&... args ) -> Dc&;

        auto handle() const -> HDC  { return m_handle; }
        operator HDC() const        { return handle(); }

        class Selection;                                    // RAII for SelectObject, separate.
    };

    inline Dc::~Dc() {}

    template< class... Args >
    inline auto Dc::use( const Args&... colors )
        -> Dc&
    {
        (colors.set_in( m_handle ), ...);
        return *this;
    }
    
    inline auto Dc::fill( const RECT& area )
        -> Dc&
    {
        FillRect( m_handle, &area, 0 );
        return *this;
    }
    
    template< class Api_func, class... Args >
    inline auto Dc::draw( const Api_func api_func, const Args&... args )
        -> Dc&
    {
        api_func( m_handle, args... );
        return *this;
    }

    class Window_dc: public Dc
    {
        HWND    m_window;

    public:
        // Cleanup is intentionally also done for a zero handle, which represents the screen.
        ~Window_dc() override { ReleaseDC( m_window, handle() ); }
        Window_dc( const HWND window ): Dc( GetDC( window ) ), m_window( window ) {}
    };


    class Screen_dc: public Window_dc
    {
    public:        
        Screen_dc(): Window_dc( 0 ) {}                      // Main screen specified implicitly.
    };

    // Instantiate this in response to a `WM_PAINT` message.
    class Client_rect_dc: public Dc
    {
        HWND            m_window;
        PAINTSTRUCT     m_paint_info;

    public:
        ~Client_rect_dc() override
        {
            EndPaint( m_window, &m_paint_info );
        }
        
        Client_rect_dc( const HWND window ):
            Dc( BeginPaint( window, &paint_info ) ),
            m_window( window )
        {}
        
        auto info() const -> const PAINTSTRUCT& { return m_paint_info; }
    };


    class Memory_dc: public Dc
    {
    public:
        ~Memory_dc() override { DeleteDC( handle() ); }
        Memory_dc(): Dc( CreateCompatibleDC( 0 ) ) {}       // Implicitly DC for main screen.
    };


    inline auto bitmap_in( const HDC dc )
        -> HBITMAP
    { return static_cast<HBITMAP>( GetCurrentObject( dc, OBJ_BITMAP ) ); }

    class Bitmap_dc: public Memory_dc
    {
    public:
        Bitmap_dc( const HBITMAP bitmap ):
            Memory_dc()
        {
            SelectObject( handle(), bitmap );
        }
        
        auto bitmap() const -> HBITMAP { return bitmap_in( handle() ); }
    };
}  // namespace winapi::gdi
