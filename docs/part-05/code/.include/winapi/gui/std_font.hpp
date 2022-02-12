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

    inline const auto std_font = Standard_font();       // Converts implicitly to HFONT.

    inline void set_font( const HWND window, const HFONT font )
    {
        const auto callback = []( const HWND control, const LPARAM font ) noexcept -> BOOL
        {
            SetWindowFont( control, reinterpret_cast<HFONT>( font ), true );
            return true;
        };

        SetWindowFont( window, font, true );
        EnumChildWindows( window, callback, reinterpret_cast<LPARAM>( font ) );
    }

    inline void set_standard_font( const HWND window )
    {
        set_font( window, std_font );
    }
}  // namespace winapi::gui
