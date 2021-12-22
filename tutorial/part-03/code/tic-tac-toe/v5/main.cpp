// v5 - Keyboard interaction.
// v4 - Gross imperfections fixed: Windows standard GUI font; turned off topmost mode;
//      modern look ’n feel via application manifest resource and initcontrolsex.
// v3 - Refactoring: <windows.h> wrapped; using <windowsx.h> macros; resource-id class.
//      <winapi_util.hpp> introduced as place for simple Windows API utility stuff.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimum code to display a window based on a dialog template resource.

#include "winapi_util.hpp"          // HANDLER_OF_WM, winapi_util::*
#include "cpp_util.hpp"             // cpp_util::*
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

namespace wu    = winapi_util;
namespace cpp   = cpp_util;

using cpp::P_;

void set_app_icon( const HWND window )
{
    wu::set_icon( window, wu::Resource_id{ IDI_APP } );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( wu::this_exe, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
}

void on_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_getdlgcode( const HWND window, const P_<MSG> p_msg )
    -> UINT
{ return DLGC_WANTARROWS; }

auto on_initdialog( const HWND window, const HWND /*focus*/, const LPARAM /*ell_param*/ )
    -> bool
{
    wu::set_standard_gui_font( window );
    wu::remove_topmost_style_for( window );
    set_app_icon( window );
    set_rules_text( window );
    return true;    // `true` sets focus to the `focus` control.
}

void on_key_down(
    const HWND          window,
    const UINT          vk,
    const int           repeat_count,
    const UINT          flags
    )
{
    MessageBox( window, "key_down", "poI", MB_ICONINFORMATION );
}

void on_key(
    const HWND          window,
    const UINT          vk,
    const bool          is_down,
    const int           repeat_count,
    const UINT          flags
    )
{ if( is_down or true ) { on_key_down( window, vk, repeat_count, flags ); } }

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG params = {window, msg_id, w_param, ell_param};   // Used by HANDLER_OF_WM.
    switch( msg_id ) {
        case WM_CLOSE:      return HANDLER_OF_WM( CLOSE, params, on_close );
        case WM_GETDLGCODE: return HANDLER_OF_WM( GETDLGCODE, params, on_getdlgcode );
        case WM_INITDIALOG: return HANDLER_OF_WM( INITDIALOG, params, on_initdialog );
        case WM_KEYUP:      return HANDLER_OF_WM( KEYUP, params, on_key );
        case WM_KEYDOWN:    return HANDLER_OF_WM( KEYDOWN, params, on_key );
    }
    return false;   // Didn't process the message, want default processing.
}

auto main() -> int
{
    wu::init_common_controls();
    DialogBox(
        wu::this_exe, wu::Resource_id{ IDD_MAIN_WINDOW }.as_ptr(),
        HWND(),             // Parent window, a zero handle is "no parent".
        message_handler
        );
}
