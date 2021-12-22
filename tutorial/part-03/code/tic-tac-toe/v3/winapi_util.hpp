#pragma once
#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    
#include <windowsx.h>               // E.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG

#include <initializer_list>         // For using range-based `for` over initializer list.

namespace winapi_util {

    // Invokes various <windowsx.h> “message cracker” macros like `HANDLE_WM_CLOSE`. Each such
    // macro interprets WPARAM and LPARAM depending on the message id, and in turn invokes a
    // specified handler func with message dependent arguments created from WPARAM and LPARAM.
    #define HANDLER_OF_WM( msg_name, m, handler_func ) \
        HANDLE_WM_##msg_name( m.hwnd, m.wParam, m.lParam, handler_func )

    inline const HINSTANCE this_exe = GetModuleHandle( nullptr );

    struct Icon_size{ enum Enum{ small = ICON_SMALL, big = ICON_BIG }; };

    struct Resource_id
    {
        int value;
        auto as_ptr() const -> const char* { return MAKEINTRESOURCE( value ); }
    };

    inline void set_icon( const HWND window, const Icon_size::Enum size, const Resource_id id )
    {
        const int       pixel_size  = (size == Icon_size::small? 16 : 32);
        const HANDLE    icon        = LoadImage(
            this_exe, id.as_ptr(), IMAGE_ICON, pixel_size, pixel_size, {}
            );

        SendMessage( window, WM_SETICON, size, reinterpret_cast<LPARAM>( icon ) );
    }

    inline void set_icon( const HWND window, const Resource_id id )
    {
        for( const auto icon_size: {Icon_size::small, Icon_size::big} ) {
            set_icon( window, icon_size, id );
        }
    }

}  // namespace winapi_util
