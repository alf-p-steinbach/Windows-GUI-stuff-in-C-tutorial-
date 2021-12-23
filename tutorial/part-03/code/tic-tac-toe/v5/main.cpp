// v5 - Basic interaction.
// v4 - Gross imperfections fixed: Windows standard GUI font; turned off topmost mode;
//      modern look ’n feel via application manifest resource and initcontrolsex.
// v3 - Refactoring: <windows.h> wrapped; using <windowsx.h> macros; resource-id class.
//      <winapi_util.hpp> introduced as place for simple Windows API utility stuff.
// v2 - Missing window parts added programmatically: the rules text; the window icon.
// v1 - Roughly minimum code to display a window based on a dialog template resource.

#include "winapi_util.hpp"          // HANDLER_OF_WM, winapi_util::*
#include "cpp_util.hpp"
#include "resources.h"              // IDS_RULES, IDC_RULES_DISPLAY, IDD_MAIN_WINDOW

#include <assert.h>

#include <array>
#include <optional>
#include <string>

namespace wu    = winapi_util;
namespace cu    = cpp_util;

using   cu::squared, cu::Range, cu::is_in;
using   std::array, std::optional, std::to_string;

struct Cell
{
    struct State
    {
        enum Enum{ empty, cross, circle };
        
        static auto opposite_of( const Enum state )
            -> Enum
        { return array{ empty, circle, cross }[state]; }
    };
};

struct Board
{
    enum{ size = 3, n_cells = squared( size ), max_index = n_cells - 1 };

    struct Line{ int start; int stride; };
    static constexpr Line lines[] =
    {
        {0, 1}, {3, 1}, {6, 1}, {0, 3}, {1, 3}, {2, 3}, {0, 4}, {2, 2}
    };

    array<Cell::State::Enum, n_cells>   cells   = {};
    
    auto win_line_with( const Cell::State::Enum state ) const
        -> optional<Line>
    {
        for( const Line& line: lines ) {
            int count = 0;
            for( int offset = 0; offset < size*line.stride; offset += line.stride ) {
                count += (cells[line.start + offset] == state);
            }
            if( count == size ) { return line; }
        }
        return {};
    }
};

struct Game
{
    Board                   board       = {};
    int                     n_moves     = 0;
    optional<Board::Line>   win_line    = {};

    void note_any_win( const Cell::State::Enum state )
    {
        if( const auto new_win_line = board.win_line_with( state ) ) {
            win_line = new_win_line;
            return;
        }
    }
    
    auto is_over() const -> bool { return n_moves == Board::n_cells or win_line; }
    
    auto find_computer_move() const
        -> int
    {
        assert( not is_over() );
        // Choose direct win if possible.
        for( int i = 0; i < Board::n_cells; ++i ) {
            if( board.cells[i] == Cell::State::empty ) {
                Board a_copy = board;
                a_copy.cells[i] = Cell::State::circle;
                if( a_copy.win_line_with( Cell::State::circle ) ) {
                    return i;
                }
            }
        }
        // Else block user win if any.
        for( int i = 0; i < Board::n_cells; ++i ) {
            if( board.cells[i] == Cell::State::empty ) {
                Board a_copy = board;
                a_copy.cells[i] = Cell::State::cross;
                if( a_copy.win_line_with( Cell::State::cross ) ) {
                    return i;
                }
            }
        }
        // Else choose a move at random.
        const int n_possibles = Board::n_cells - n_moves;
        const int which_free_cell = cu::random_up_to( n_possibles );
        int count = 0;
        for( int i = 0; i < Board::n_cells; ++i ) {
            if( board.cells[i] == Cell::State::empty ) {
                if( count == which_free_cell ) {
                    return i;
                }
                ++count;
            }
        }
        for( ;; );  // Should never get here.
    }
};

Game the_game;

auto button_for_cell_index( const int i, const HWND window )
    -> HWND
{ return GetDlgItem( window, BOARD_BUTTON_BASE + i + 1 ); }

void mark_if_win( const HWND window )
{
    if( the_game.win_line ) {
        const Board::Line& wl = the_game.win_line.value();
        for( int offset = 0; offset < Board::size*wl.stride; offset += wl.stride ) {
            const int win_i = wl.start + offset;
            (void) win_i; (void) window; // TODO: Image? Bold font?
        }
    }
}

void on_user_move( const HWND window, const int user_move )
{
    if( the_game.board.cells[user_move] != Cell::State::empty or the_game.is_over() ) {
        FlashWindow( window, true );    // Documentation per late 2021 is misleading/wrong.
        return;
    }

    the_game.board.cells[user_move] = Cell::State::cross;
    the_game.note_any_win( Cell::State::cross );
    ++the_game.n_moves;
    SetWindowText( button_for_cell_index( user_move, window ), "X" );
    if( not the_game.is_over() ) {
        const int computer_move = the_game.find_computer_move();
        the_game.board.cells[computer_move] = Cell::State::circle;
        the_game.note_any_win( Cell::State::circle );
        ++the_game.n_moves;
        SetWindowText( button_for_cell_index( computer_move, window ), "O" );
    }
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
    const int cell_1_id = BOARD_BUTTON_BASE + 1;
    const int cell_9_id = BOARD_BUTTON_BASE + 9;
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
    return true;    // `true` sets focus to the `focus` control.
}

auto CALLBACK message_handler(
    const HWND      window,
    const UINT      msg_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    const MSG params = {window, msg_id, w_param, ell_param};   // Used by HANDLER_OF_WM.
    switch( msg_id ) {
        case WM_CLOSE:      return HANDLER_OF_WM( CLOSE, params, on_wm_close );
        case WM_COMMAND:    return HANDLER_OF_WM( COMMAND, params, on_wm_command );
        case WM_INITDIALOG: return HANDLER_OF_WM( INITDIALOG, params, on_wm_initdialog );
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
