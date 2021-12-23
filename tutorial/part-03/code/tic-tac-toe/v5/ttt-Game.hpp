#pragma once
#include "cpp_util.hpp"

#include <array>
#include <optional>

namespace ttt {
    namespace cu    = cpp_util;

    using   cu::squared;
    using   std::array, std::optional;

    struct Cell{ struct State{ enum Enum{ empty, cross, circle }; }; };

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
            // Choose a direct computer win if possible.
            for( int i = 0; i < Board::n_cells; ++i ) {
                if( board.cells[i] == Cell::State::empty ) {
                    Board a_copy = board;
                    a_copy.cells[i] = Cell::State::circle;
                    if( a_copy.win_line_with( Cell::State::circle ) ) {
                        return i;
                    }
                }
            }
            // Else block the user’s win if any.
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
            const int which_free_cell = 1 + cu::random_up_to( n_possibles );
            int count = 0;
            for( int i = 0; i < Board::n_cells; ++i ) {
                if( board.cells[i] == Cell::State::empty ) {
                    ++count;
                    if( count == which_free_cell ) {
                        return i;
                    }
                }
            }
            for( ;; );  // Should never get here.
        }
    };
}  // namespace ttt
