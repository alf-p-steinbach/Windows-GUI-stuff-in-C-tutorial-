#pragma once
#include <wrapped-windows-h.hpp>        // Safer and faster. Safe = e.g. no `small` macro.
#include <cpp_support_machinery.hpp>    // cpp_support_machinery::contains_

#include <optional>     // std::optional

namespace winapi_support_machinery:: dialog_message {
    namespace csm = cpp_support_machinery;
    using   csm::contains_;
    using   std::optional;

    using Result = optional<INT_PTR>;
    constexpr auto processed = Result( 0 );     // 0 is a dummy, could be any value.
    
    inline auto uses_dlgproc_return_value( const unsigned msg_id )
        -> bool
    {
        static const unsigned msgs[] = 
        {
            WM_CTLCOLORMSGBOX, WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX, WM_CTLCOLORBTN, WM_CTLCOLORDLG,
            WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_COMPAREITEM, WM_VKEYTOITEM, WM_CHARTOITEM,
            WM_QUERYDRAGICON, WM_INITDIALOG
        };

        return contains_( msg_id, msgs );
    }

    using Message_handler_func = auto( const MSG& ) -> Result;

    template< Message_handler_func message_handler >
    inline auto CALLBACK callback_(
        const HWND      window,
        const UINT      msg_id,
        const WPARAM    w_param,
        const LPARAM    ell_param
        ) -> INT_PTR
    {
        const Result result = message_handler({ window, msg_id, w_param, ell_param });
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
}  // namespace winapi_support_machinery:: dialog_message
