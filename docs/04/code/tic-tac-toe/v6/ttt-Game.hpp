#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <cpp/util.hpp>

#include <array>
#include <initializer_list>
#include <optional>

namespace ttt {
    namespace cu = cpp::util;
    using   std::array, std::optional;

    namespace cell_state {
        enum Enum{ empty, cross, circle };
    }  // namespace cell_state

    struct Board
    {
        enum{ size = 3, n_cells = cu::squared( size ), max_index = n_cells - 1 };

        // x is left to right, y is bottom to top, zero-based, i = 3*y + x.
        struct Line{ int start; int stride; };  // `start` in left col or bottom row.
        static constexpr Line lines[] =
        {
            {0, 1}, {3, 1}, {6, 1}, {0, 3}, {1, 3}, {2, 3}, {0, 4}, {2, 2}
        };

        array<cell_state::Enum, n_cells>    cells   = {};
        
        auto win_line_with( const cell_state::Enum state ) const
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
        using Opt_line = optional<Board::Line>;

        Board       board       = {};
        int         n_moves     = 0;
        Opt_line    win_line    = {};

        void store_any_win_line_with( const cell_state::Enum state )
        {
            if( const Opt_line new_win_line = board.win_line_with( state ) ) {
                win_line = new_win_line;
            }
        }
        
        auto is_over() const -> bool { return n_moves == Board::n_cells or win_line; }
        
        void make_move( const int cell_index )
        {
            assert( not is_over() );
            assert( board.cells[cell_index] == cell_state::empty );

            const auto new_state = (n_moves % 2 == 0? cell_state::cross : cell_state::circle);
            board.cells[cell_index] = new_state;
            store_any_win_line_with( new_state );
            ++n_moves;
        }

        auto find_computer_move() const
            -> int
        {
            assert( not is_over() );
            for( const auto state_to_check: {cell_state::circle, cell_state::cross} ) {
                // If state is cell_state::circle: Choose a direct computer win if possible.
                // Else state is cell_state::cross:  Block the user’s win if any.
                for( int i = 0; i < Board::n_cells; ++i ) {
                    if( board.cells[i] == cell_state::empty ) {
                        Board a_copy = board;
                        a_copy.cells[i] = state_to_check;
                        if( a_copy.win_line_with( state_to_check ) ) {
                            return i;
                        }
                    }
                }
            }
            
            // Else choose a move at random.
            const int n_possibles = Board::n_cells - n_moves;
            const int which_free_cell = cu::random_in( {1, n_possibles} );
            int count = 0;
            for( int i = 0; i < Board::n_cells; ++i ) {
                if( board.cells[i] == cell_state::empty ) {
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
