#include <windows.h>
#include <windowsx.h>       // HANDLE_WM_CLOSE, HANDLE_WM_INITDIALOG
#include "resources.h"      // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

//------------------------------------------------ Support machinery:

// Invokes various <windowsx.h> macros that in turn invoke specified message handler funcs:
#define CALL_HANDLER( msg_name, handler_func ) \
    HANDLE_##msg_name( ch_params.hwnd, ch_params.wParam, ch_params.lParam, handler_func )

struct Standard_gui_font
{
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

const auto      std_gui_font    = Standard_gui_font();

// A Windows 11 workaround hack. The window is assumed to presently be a “topmost” window.
// The effect is to bring the window to the top of ordinary window z-order.
void remove_topmost_style_for( const HWND window )
{
    SetWindowPos( window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

void set_standard_gui_font( const HWND window )
{
    const auto callback = []( HWND control, LPARAM ) -> BOOL
    {
        SetWindowFont( control, std_gui_font.handle, true );
        return true;
    };

    SetWindowFont( window, std_gui_font.handle, true );
    EnumChildWindows( window, callback, 0 );
}


//------------------------------------------------ App:

const HINSTANCE this_exe        = GetModuleHandle( nullptr );

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_initdialog( const HWND window, const HWND focus, const LPARAM ell_param )
    -> bool
{
    (void) focus;  (void) ell_param;
    
    remove_topmost_style_for( window );
    set_standard_gui_font( window );

    // Init rules_display
    char text[2048];
    LoadString( this_exe, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
    return true;    // `true` sets focus to the control specified by the `focus` param.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG ch_params = {window, msg_id, w_param, ell_param};
    switch( msg_id ) {
        case WM_CLOSE:          return CALL_HANDLER( WM_CLOSE, &on_close );
        case WM_INITDIALOG:     return CALL_HANDLER( WM_INITDIALOG, on_initdialog );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    DialogBox( this_exe, MAKEINTRESOURCE( IDD_MAIN_WINDOW ), HWND(), &message_handler );
}
