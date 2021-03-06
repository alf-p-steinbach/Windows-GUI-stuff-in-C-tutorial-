#pragma once
#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    
#include <windowsx.h>               // E.g. HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include <commctrl.h>               // InitCommonControlsEx

#include <initializer_list>         // For using range-based `for` over initializer list.

namespace winapi_util {
    inline const HINSTANCE this_exe = GetModuleHandle( nullptr );

    struct Resource_id
    {
        int value;
        auto as_ptr() const -> const char* { return MAKEINTRESOURCE( value ); }
    };

    namespace icon_sizes{
        enum Enum{ small = ICON_SMALL, large = ICON_BIG };
    }  // namespace icon_sizes

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

    struct Standard_gui_font
    {
        Standard_gui_font( const Standard_gui_font& ) = delete;
        auto operator=( const Standard_gui_font& ) -> Standard_gui_font& = delete;

        HFONT   handle;

        ~Standard_gui_font()
        {
            DeleteFont( handle );
        }

        Standard_gui_font()
        {
            // Get the system message box font
            const auto ncm_size = sizeof( NONCLIENTMETRICS );
            NONCLIENTMETRICS metrics = {ncm_size};
            SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm_size, &metrics, 0 );
            handle = CreateFontIndirect( &metrics.lfMessageFont );
        }
    };

    inline const auto   std_gui_font    = Standard_gui_font();

    inline void set_standard_gui_font( const HWND window )
    {
        const auto callback = []( HWND control, LPARAM ) -> BOOL
        {
            SetWindowFont( control, std_gui_font.handle, true );
            return true;
        };

        SetWindowFont( window, std_gui_font.handle, true );
        EnumChildWindows( window, callback, 0 );
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

}  // namespace winapi_util
