﻿// v5 - Basic interaction (working game play, notification of win/lose/tie, restart).
// v4 - Gross imperfections fixed: Windows standard GUI font; turned off topmost mode;
//      modern look ’n feel via application manifest resource and initcontrolsex.
// v3 - Refactoring: <windows.h> wrapped; using <windowsx.h> macros; resource-id class.
//      <winapi_util.hpp> introduced as place for simple Windows API utility stuff.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimum code to display a window based on a dialog template resource.

#include "ttt-Game.hpp"             // ttt::Game
#include "winapi_util.hpp"          // HANDLER_OF_WM, winapi_util::*
#include "cpp_util.hpp"
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

#include <assert.h>
#include <string>

namespace wu    = winapi_util;
namespace cu    = cpp_util;

using   cu::Range, cu::is_in;
using   std::string, std::to_string;
using   ttt::Cell, ttt::Board, ttt::Game;

constexpr int cell_1_id = BOARD_BUTTON_BASE + 1;
constexpr int cell_9_id = BOARD_BUTTON_BASE + 9;

constexpr auto empty    = Cell::State::empty;
constexpr auto cross    = Cell::State::cross;
constexpr auto circle   = Cell::State::circle;

Game    the_game;
string  the_original_status_text;       // Initialized by `on_wm_initdialog`.

void set_status_text( const HWND window, const string& text )
{
    const HWND status_display = GetDlgItem( window, IDC_STATUS_DISPLAY );
    SetWindowText( status_display, text.c_str() );
}

auto button_for_cell_index( const int i, const HWND window )
    -> HWND
{ return GetDlgItem( window, BOARD_BUTTON_BASE + i + 1 ); }

void make_a_new_game( const HWND window )
{
    the_game = {};
    for( int i = 1; i <= 9; ++i ) {
        const HWND control = button_for_cell_index( i - 1, window );
        const string text = "&" + to_string( i );
        SetWindowText( control, text.c_str() );
        wu::enable( control );
    }
    wu::enable( GetDlgItem( window, IDC_RULES_DISPLAY ) );
    SetFocus( GetDlgItem( window, BOARD_BUTTON_BASE + 7 ) );
    set_status_text( window, the_original_status_text );
}

void indicate_game_over( const HWND window )
{
    for( int id = cell_1_id; id <= cell_9_id; ++id ) {
        wu::disable( GetDlgItem( window, id ) );
    }
    wu::disable( GetDlgItem( window, IDC_RULES_DISPLAY ) );
    if( the_game.win_line ) {
        const bool user_won = (the_game.board.cells[the_game.win_line->start] == cross);
        if( user_won ) {
            set_status_text( window, "You won! Yay! Click anywhere for a new game." );
        } else {
            set_status_text( window, "I won. Better luck next time. Just click anywhere." );
        }
    } else {
        set_status_text( window, "It's a tie.  Click anywhere for a new game." );
    }
}

void on_user_move( const HWND window, const int user_move )
{
    if( the_game.board.cells[user_move] != empty or the_game.is_over() ) {
        FlashWindow( window, true );    // Documentation per late 2021 is misleading/wrong.
        return;
    }

    the_game.board.cells[user_move] = cross;
    the_game.note_any_win( cross );
    ++the_game.n_moves;
    SetWindowText( button_for_cell_index( user_move, window ), "X" );
    if( not the_game.is_over() ) {
        const int computer_move = the_game.find_computer_move();
        the_game.board.cells[computer_move] = circle;
        the_game.note_any_win( circle );
        ++the_game.n_moves;
        SetWindowText( button_for_cell_index( computer_move, window ), "O" );
    }
    if( the_game.is_over() ) { indicate_game_over( window ); }
}

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

void on_wm_close( const HWND window )
{
    EndDialog( window, IDOK );
}

void on_wm_command( const HWND window, const int id, const HWND control, const UINT notification )
{
    if( is_in( Range{ cell_1_id, cell_9_id }, id ) and notification == BN_CLICKED ) {
        const int cell_index = id - cell_1_id;
        on_user_move( window, cell_index );
    }
    (void) control;
}

auto on_wm_initdialog( const HWND window, const HWND /*focus*/, const LPARAM /*ell_param*/ )
    -> bool
{
    wu::set_standard_gui_font( window );
    wu::remove_topmost_style_for( window );
    set_app_icon( window );
    set_rules_text( window );
    the_original_status_text = wu::text_of( GetDlgItem( window, IDC_STATUS_DISPLAY ) );
    return true;    // `true` sets focus to the `focus` control.
}

void on_wm_lbuttondown(
    const HWND          window,
    const bool          /*is_double_click*/,
    const int           /*x*/,
    const int           /*y*/,
    const unsigned      /*key_flags*/
    )
{
    if( the_game.is_over() ) {
        make_a_new_game( window );
    }
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG params = {window, msg_id, w_param, ell_param};
    switch( msg_id ) {
        case WM_CLOSE:          return HANDLER_OF_WM( CLOSE, params, on_wm_close );
        case WM_COMMAND:        return HANDLER_OF_WM( COMMAND, params, on_wm_command );
        case WM_INITDIALOG:     return HANDLER_OF_WM( INITDIALOG, params, on_wm_initdialog );
        case WM_LBUTTONDOWN:    return HANDLER_OF_WM( LBUTTONDOWN, params, on_wm_lbuttondown );
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