#pragma once
#include <wrapped-windows-h.hpp>        // Safer and faster. Safe = e.g. no `small` macro.
#include <cpp_support_machinery.hpp>
#include <winapi_support_machinery/resource.hpp>

#include <assert.h>                     // assert

#include <array>                        // std::array
#include <initializer_list>             // For using range-based `for` over initializer list.

namespace winapi_support_machinery:: icon {
    namespace csm = cpp_support_machinery;

    using   csm::contains, csm::Int_range;
    using   std::array;

    namespace size_names {
        // Respectively 0 and 1; see documentation of WM_SETICON. Note: WM_GETICON also has 2.
        enum Enum{ small = ICON_SMALL, large = ICON_BIG };      // get_app_small = ICON_SMALL2
        static_assert( small == 0 and large == 1 );
    }  // namespace sizes

    constexpr auto n_pixels_for( const size_names::Enum name )
        -> int
    { return array{ 16, 32 }.at( name ); }

    inline auto in( const HWND window, const size_names::Enum size_name )
        -> HICON
    {
        const LRESULT r = SendMessage( window, WM_GETICON, size_name, 0 );
        return reinterpret_cast<HICON>( r );
    }

    inline auto set_in(
        const HWND                  window,
        const size_names::Enum      size_name,
        const HICON                 icon
        ) -> HICON
    {
        const LRESULT r = SendMessage(
            window, WM_SETICON, size_name, reinterpret_cast<LPARAM>( icon )
            );
        return reinterpret_cast<HICON>( r );
    }

    inline auto set_in(
        const HWND                  window,
        const size_names::Enum      size_name,
        const resource::Location&   location
        ) -> HICON
    {
        const int       pixel_size  = n_pixels_for( size_name );
        const HANDLE    icon        = LoadImage(
            location.module(), location.id().as_pseudo_ptr(),
            IMAGE_ICON,
            pixel_size, pixel_size, {}
            );
        return set_in( window, size_name, static_cast<HICON>( icon ) );
    }

    inline void set_in( const HWND window, const resource::Location& location )
    {
        for( const auto size_name: {size_names::small, size_names::large} ) {
            set_in( window, size_name, location );
        }
    }
}  // namespace winapi_support_machinery:: icon
