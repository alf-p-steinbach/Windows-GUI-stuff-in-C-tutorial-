// v3-2 - Support for general message processing results in dialog proc.
// v3-1 - Refactoring: <windows.h> wrapped.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW, ...
#include "wrapped-windows-h.hpp"    // Safer and faster. Safe = e.g. no `small` macro.    

#include <stdlib.h>                 // EXIT_...

#include <algorithm>                // std::find
#include <iterator>                 // std::(begin, end)
#include <optional>                 // std::optional

using   std::find, std::begin, std::end, std::optional;
using Char_ptr = const char*;


//------------------------------------------- CPP support machinery:

template< class Value, class Sequence >
auto contains( const Value& v, const Sequence& seq )
    -> bool
{
    const auto it_first     = begin( seq );
    const auto it_beyond    = end( seq );
    return (find( it_first, it_beyond, v ) != it_beyond);
}


//------------------------------------------- API support machinery:

const HINSTANCE this_executable = GetModuleHandle( nullptr );

namespace icon_sizes{
    enum Enum{ small = ICON_SMALL, large = ICON_BIG };       // WM_SETICON values.
}  // namespace icon_sizes

void set_icon( const HWND window, const icon_sizes::Enum size, const int resource_id )
{
    const Char_ptr  id_as_pseudo_ptr    = MAKEINTRESOURCE( resource_id );
    const int       pixel_size          = (size == icon_sizes::small? 16 : 32);
    const HANDLE    icon                = LoadImage(
        this_executable, id_as_pseudo_ptr, IMAGE_ICON, pixel_size, pixel_size, {}
        );
    SendMessage( window, WM_SETICON, size, reinterpret_cast<LPARAM>( icon ) );
}

namespace dialog_message {
    using Result = optional<INT_PTR>;
    constexpr auto was_processed_indication = Result( 0 );  // 0 is a dummy, could be any value.
    
    auto uses_dlgproc_return_value( const unsigned msg_id )
        -> bool
    {
        static const unsigned msgs[] = 
        {
            WM_CTLCOLORMSGBOX, WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX, WM_CTLCOLORBTN, WM_CTLCOLORDLG,
            WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_COMPAREITEM, WM_VKEYTOITEM, WM_CHARTOITEM,
            WM_QUERYDRAGICON, WM_INITDIALOG
        };

        return contains( msg_id, msgs );
    }
}  // namespace dialog_message 


//------------------------------------------- App code:

void set_app_icon( const HWND window )
{
    set_icon( window, icon_sizes::small, IDI_APP );
    set_icon( window, icon_sizes::large, IDI_APP );
}

void set_rules_text( const HWND window )
{
    char text[2048];
    LoadString( this_executable, IDS_RULES, text, sizeof( text ) );
    const HWND rules_display = GetDlgItem( window, IDC_RULES_DISPLAY );
    SetWindowText( rules_display, text );
}

void on_wm_close( const HWND window )
{
    EndDialog( window, IDOK );
}

auto on_wm_initdialog( const HWND window )
    -> bool
{
    set_app_icon( window );
    set_rules_text( window );
    return true;    // `true` sets focus to the control specified by the `w_param`.
}

auto message_handler( const MSG& msg )
    -> optional<INT_PTR>
{
    const auto& processed = dialog_message::was_processed_indication;
    switch( msg.message ) {
        case WM_CLOSE:          on_wm_close( msg.hwnd ); return processed;
        case WM_INITDIALOG:     return on_wm_initdialog( msg.hwnd );
    }
    return {};
}

auto CALLBACK dlgproc(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const optional<INT_PTR> result = message_handler({ window, msg_id, w_param, ell_param });
    if( not result.has_value() ) {
        return false;                   // Message was not processed, want default processing.
    } else if( dialog_message::uses_dlgproc_return_value( msg_id ) ) {
        return result.value();          // Special message was processed, this is the result.
    } else {
        // This may needlessly call `SetWindowLongPtr` for e.g. `WM_CLOSE`, but that's OK.
        // Avoiding that would require a more complex structure than just `optional<INT_PTR>`.
        SetWindowLongPtr( window, DWLP_MSGRESULT, result.value() ); // This is the result.
        return true;                    // General message was processed.
    }
}

auto main() -> int
{
    const Char_ptr  resource_id_as_pseudo_ptr   = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    // Note: that there /is/ a return value is undocumented. Can fail if no dialog resource.
    const auto return_value = DialogBox(
        this_executable, resource_id_as_pseudo_ptr, HWND( 0 ), dlgproc
        );
    return (return_value <= 0? EXIT_FAILURE : EXIT_SUCCESS);
}
