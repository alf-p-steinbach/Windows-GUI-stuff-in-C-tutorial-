#include <cpp/util.hpp>                         // cpp::util::(hopefully, No_copying, Types_)
#include <winapi/gdi/color-usage-classes.hpp>   // winapi::gdi::(Brush_color, Pen_color, Gap_color)
#include <winapi/gui/std_font.hpp>              // winapi::gui::std_font
#include <wrapped-winapi/windows-h.hpp>

#include <stddef.h>         // size_t

#include <type_traits>      // std::is_same_v
#include <utility>          // std::(index_sequence, 

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying, cu::Types_;

    inline void make_practical( const HDC dc )
    {
        SelectObject( dc, GetStockObject( DC_PEN ) );
        SelectObject( dc, GetStockObject( DC_BRUSH ) );
        SetBkMode( dc, TRANSPARENT );   // Don't fill in background of text, please.
        SelectObject( dc, gui::std_font );
    }

    class Dc: No_copying
    {
        HDC     m_handle;

        // Expands a specified RECT argument into its four member values as arguments.
        template<
            size_t      rect_arg_index,
            class       Api_func,
            class...    Args,
            size_t...   indices_before_rect,
            size_t...   indices_after_rect       // 0-based, i.e. minus offset
            >
        void call_draw_with_rect_arg_expanded_(
            const Api_func                              api_func,
            const std::tuple<const Args&...>&           args_tuple,
            std::index_sequence<indices_before_rect...> ,
            std::index_sequence<indices_after_rect...>
            ) const
        {
            const RECT& r = std::get<rect_arg_index>( args_tuple );
            draw(
                api_func,
                std::get<indices_before_rect>( args_tuple )...,     // Can be empty, works.
                r.left, r.top, r.right, r.bottom,
                std::get< rect_arg_index + 1 + indices_after_rect >( args_tuple )...
                );
        }

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
        auto use( const Args&... colors ) const
            -> const Dc&
        {
            (colors.set_in( m_handle ), ...);
            return *this;
        }
        
        auto fill( const RECT& area ) const
            -> const Dc&
        {
            FillRect( m_handle, &area, 0 );
            return *this;
        }
        
        template< class Api_func, class... Args >
        auto simple_draw( const Api_func api_func, const Args&... args ) const
            -> const Dc&
        {
            api_func( m_handle, args... );
            return *this;
        }

        template< class Api_func, class... Args >
        auto draw( const Api_func api_func, const Args&... args ) const
            -> const Dc&
        {
            const int i_first_rect = Types_< Args... >::template index_of_first_< RECT >;
            if constexpr( i_first_rect < 0 ) {
                api_func( m_handle, args... );
            } else {
                call_draw_with_rect_arg_expanded_<i_first_rect>(
                    api_func,
                    std::tie( args... ),
                    std::make_index_sequence<i_first_rect>(),
                    std::make_index_sequence<sizeof...( Args ) - (i_first_rect + 1)>()
                    );
            }
            return *this;
        }

        auto handle() const -> HDC { return m_handle; }
        operator HDC() const { return handle(); }

        class Selection;                                    // RAII for SelectObject, separate.
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
