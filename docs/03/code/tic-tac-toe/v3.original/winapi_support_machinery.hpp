#pragma once
#include "wrapped-windowsx-h.hpp"   // The "x" = e.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG

#include <initializer_list>         // For using range-based `for` over initializer list.

#define WSM_HANDLE_WM( name, handler_func ) \
    HANDLE_WM_##name( msg.hwnd, msg.wParam, msg.lParam, handler_func )
// E.g., `WSM_HANDLE_WM( CLOSE, ...` expands to `HANDLE_WM_CLOSE( ...`, which calls the
// <windowsx.h> macro HANDLE_WM_CLOSE that in turns calls the specified `WM_CLOSE` handler
// with appropriate arguments extracted from the data in a `MSG` variable called `msg`.

namespace winapi_support_machinery {
    inline const HINSTANCE this_executable = GetModuleHandle( nullptr );

    namespace icon_sizes{
        enum Enum{ small = ICON_SMALL, large = ICON_BIG };
    }  // namespace icon_sizes

    struct Resource_id
    {
        int value;

        auto as_pseudo_ptr() const
            -> const char*
        { return MAKEINTRESOURCE( value ); }
    };

    inline void set_icon(
        const HWND              window,
        const icon_sizes::Enum  size,
        const Resource_id       id
        )
    {
        const int       pixel_size  = (size == icon_sizes::small? 16 : 32);
        const HANDLE    icon        = LoadImage(
            this_executable, id.as_pseudo_ptr(), IMAGE_ICON, pixel_size, pixel_size, {}
            );

        SendMessage( window, WM_SETICON, size, reinterpret_cast<LPARAM>( icon ) );
    }

    inline void set_icon( const HWND window, const Resource_id id )
    {
        for( const auto size: {icon_sizes::small, icon_sizes::large} ) {
            set_icon( window, size, id );
        }
    }
}  // namespace winapi_support_machinery
