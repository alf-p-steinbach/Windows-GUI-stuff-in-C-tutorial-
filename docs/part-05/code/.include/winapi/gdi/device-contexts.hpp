#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                         // cpp::util::(hopefully, No_copying, Types_)
#include <winapi/gdi/color-usage-classes.hpp>   // winapi::gdi::(Brush_color, Pen_color, Gap_color)
#include <winapi/gui/std_font.hpp>              // winapi::gui::std_font
#include <wrapped-winapi/windows-h.hpp>         // General Windows API.

#include <stddef.h>         // size_t

#include <tuple>            // std::(get, tie, tuple)
#include <type_traits>      // std::is_same_v
#include <utility>          // std::(index_sequence, 

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::No_copying, cu::Types_;
    using   std::get, std::tie, std::tuple,
            std::index_sequence,
            std::make_index_sequence;

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

        // Internal helper for expanding a RECT argument into its 4 member values as arguments.
        template< class Api_func, class... Args, size_t... i_before, size_t... i_after>
        inline void call_draw_with_rect_arg_expanded(
            const Api_func                          api_func,
            const tuple<const Args&...>&            args_tuple,
            index_sequence<i_before...>  ,
            index_sequence<i_after...>
            );

    protected:
        inline virtual ~Dc() = 0;                           // Derived-class responsibility.

        Dc( HDC&& handle, const bool do_extended_init = true ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
            if( do_extended_init ) { make_practical( m_handle ); }
        }

    public:
        template< class... Args >
        inline auto use( const Args&... colors ) -> Dc&;
     
        // Convenience special cases.
        auto bg( const Brush_color color ) -> Dc& { return use( color ); }
        auto fg( const Pen_color color ) -> Dc& { return use( color ); }
        
        inline auto fill( const RECT& area ) -> Dc&;
        
        template< class Api_func, class... Args >
        inline auto simple_draw( const Api_func f, const Args&... args ) -> Dc&;

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
    inline auto Dc::simple_draw( const Api_func api_func, const Args&... args )
        -> Dc&
    {
        api_func( m_handle, args... );
        return *this;
    }

    template<
        class       Api_func,
        class...    Args,
        size_t...   indices_before_rect,
        size_t...   indices_after_rect       // 0-based, i.e. minus offset
        >
    inline void Dc::call_draw_with_rect_arg_expanded(
        const Api_func                          api_func,
        const tuple<const Args&...>&            args_tuple,
        index_sequence<indices_before_rect...>  ,
        index_sequence<indices_after_rect...>
        )
    {
        constexpr int rect_arg_index = sizeof...( indices_before_rect );
        const RECT& r = get<rect_arg_index>( args_tuple );
        draw(
            api_func,
            get<indices_before_rect>( args_tuple )...,     // Can be empty, works.
            r.left, r.top, r.right, r.bottom,
            get< rect_arg_index + 1 + indices_after_rect >( args_tuple )...
            );
    }

    template< class Api_func, class... Args >
    inline auto Dc::draw( const Api_func api_func, const Args&... args )
        -> Dc&
    {
        const int i_first_rect = Types_< Args... >::template index_of_first_< RECT >;
        if constexpr( i_first_rect < 0 ) {
            api_func( m_handle, args... );
        } else {
            call_draw_with_rect_arg_expanded(
                api_func,
                tie( args... ),
                make_index_sequence<i_first_rect>(),
                make_index_sequence<sizeof...( Args ) - (i_first_rect + 1)>()
                );
        }
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
