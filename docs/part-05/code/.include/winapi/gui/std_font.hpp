#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <winapi/gdi/Object_.hpp>           // winapi::gdi::Font
#include <wrapped-winapi/windowsx-h.hpp>    // windows.h + SetWindowFont    

namespace winapi::gui {
    inline auto create_std_font()
        -> HFONT
    {
        // Get the system message box font
        const auto ncm_size = sizeof( NONCLIENTMETRICS );
        NONCLIENTMETRICS metrics = {ncm_size};
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm_size, &metrics, 0 );
        return CreateFontIndirect( &metrics.lfMessageFont );
    }

    class Standard_font: public gdi::Font
    {
    public:
        Standard_font(): gdi::Font( create_std_font() ) {}
    };

    inline const auto std_font = Standard_font();

    inline void set_standard_font( const HWND window )
    {
        const auto callback = []( HWND control, LPARAM ) noexcept -> BOOL
        {
            SetWindowFont( control, std_font.handle(), true );
            return true;
        };

        SetWindowFont( window, std_font.handle(), true );
        EnumChildWindows( window, callback, 0 );
    }
}  // namespace winapi::gui
