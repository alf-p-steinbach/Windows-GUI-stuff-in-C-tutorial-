#pragma once
#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    
#include <windowsx.h>               // E.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG

#include <initializer_list>         // For using range-based `for` over initializer list.

namespace winapi_util {
    inline const HINSTANCE this_exe = GetModuleHandle( nullptr );

    namespace icon_sizes{
        enum Enum{ small = ICON_SMALL, large = ICON_BIG };
    }  // namespace icon_sizes

    struct Resource_id
    {
        int value;
        auto as_ptr() const -> const char* { return MAKEINTRESOURCE( value ); }
    };

    inline void set_icon( const HWND window, const icon_sizes::Enum size, const Resource_id id )
    {
        const int       pixel_size  = (size == icon_sizes::small? 16 : 32);
        const HANDLE    icon        = LoadImage(
            this_exe, id.as_ptr(), IMAGE_ICON, pixel_size, pixel_size, {}
            );

        SendMessage( window, WM_SETICON, size, reinterpret_cast<LPARAM>( icon ) );
    }

    inline void set_icon( const HWND window, const Resource_id id )
    {
        for( const auto icon_size: {icon_sizes::small, icon_sizes::large} ) {
            set_icon( window, icon_size, id );
        }
    }
}  // namespace winapi_util

// Invokes various <windowsx.h> “message cracker” macros like `HANDLE_WM_CLOSE`. Each such
// macro interprets WPARAM and LPARAM depending on the message id, and in turn invokes a
// specified handler func with message dependent arguments created from WPARAM and LPARAM.
#define HANDLER_OF_WM( msg_name, m, handler_func ) \
    HANDLE_WM_##msg_name( m.hwnd, m.wParam, m.lParam, handler_func )
