#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.

#include <winapi/gui/std_font.hpp>          // winapi::gui::"font stuff"
#include <wrapped-winapi-headers/windowsx-h.hpp>    // E.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include <wrapped-winapi-headers/commctrl-h.hpp>    // InitCommonControlsEx

#include <initializer_list>         // For using range-based `for` over initializer list.
#include <string>

namespace winapi::gui {
    using std::string;

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

    // Supports a Windows 11 workaround hack. The window is assumed to presently be a “topmost”
    // window. The effect is then to bring the window to the top of the ordinary window Z-order.
    void remove_topmost_style_for( const HWND window )
    {
        SetWindowPos( window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    }

    constexpr DWORD basic_common_controls = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;

    inline auto init_common_controls( const DWORD which = basic_common_controls )
        -> bool
    {
        const INITCOMMONCONTROLSEX params = {sizeof( params ), which};
        return !!InitCommonControlsEx( &params );
    }

    inline void enable( const HWND window ) { EnableWindow( window, true ); }
    inline void disable( const HWND window ) { EnableWindow( window, false ); }
    
    inline auto text_of( const HWND window )
        -> string
    {
        const int buffer_size = 1 + GetWindowTextLength( window );
        auto result = string( 1u*buffer_size, '\0' );
        if( buffer_size > 0 ) {
            const int length = GetWindowText( window, &result[0], buffer_size );
            result.resize( 1u*length );
        }
        return result;
    }
}  // namespace winapi::gui
