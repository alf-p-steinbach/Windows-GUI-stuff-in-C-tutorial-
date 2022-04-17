#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <winapi/kernel/util.hpp>                   // winapi::kernel::(this_exe, Resource_id)

#include <winapi/gui/std_font.hpp>                  // winapi::gui::"font stuff"
#include <wrapped-winapi-headers/windowsx-h.hpp>    // E.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include <wrapped-winapi-headers/commctrl-h.hpp>    // InitCommonControlsEx

#include <initializer_list>         // For using range-based `for` over initializer list.
#include <string>

namespace winapi::gui {
    using kernel::this_exe, kernel::Resource_id;
    using std::string;

    // Invokes various <windowsx.h> “message cracker” macros like `HANDLE_WM_CLOSE`. Each such
    // macro interprets WPARAM and LPARAM depending on the message id, and in turn invokes a
    // specified handler func with message dependent arguments created from WPARAM and LPARAM.
    #define HANDLER_OF_WM( msg_name, m, handler_func ) \
        HANDLE_WM_##msg_name( m.hwnd, m.wParam, m.lParam, handler_func )

    // Modern Windows has five to seven standard icon sizes. Small and big are the original.
    struct Icon_size{ enum Enum{ small = ICON_SMALL, big = ICON_BIG }; };

    inline auto pixel_size_of( const Icon_size::Enum size )
        -> int
    { return (size == Icon_size::small? 16 : 32); }

    inline void set_icon( const HWND window, const Icon_size::Enum size, const Resource_id id )
    {
        const int       pixel_size  = pixel_size_of( size );
        const HANDLE    icon        = LoadImage(
            this_exe, id.as_pseudo_ptr(), IMAGE_ICON, pixel_size, pixel_size, {}
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
    inline void remove_topmost_style_for( const HWND window )
    {
        SetWindowPos( window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    }

    inline void set_client_area_size( const HWND window, const int width, const int height )
    {
        RECT r = {0, 0, width, height};                 // Desired client area.

        const LONG window_style = GetWindowLong( window, GWL_STYLE );
        const LONG window_ex_style = GetWindowLong( window, GWL_EXSTYLE );
        const bool has_menu = (GetMenu( window ) != 0);
        
         // Find window size for given client rect.
        AdjustWindowRectEx( &r, window_style, has_menu, window_ex_style );

        SetWindowPos( window, HWND(), 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE|SWP_NOZORDER );
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
