#pragma once
#include "Board.hpp"
#include "calc.hpp"     // is_even

namespace ttt {
    struct State_definitions
    {
        enum{ max_moves = Board::n_cells };

        using Cell_state    = Board::Cell::State;
        using Position      = Board::Position;
        struct Outcome{ enum Enum{ ongoing, crosses_won, circles_won, all_cells_played }; };
    };

    class State:
        public State_definitions,       // Outcome
        private Board::Cell::State      // Makes the state names available unqualified.
    {
        Board           m_board         = {};
        int             m_n_moves       = 0;
        Outcome::Enum   m_outcome       = Outcome::ongoing;

        auto board_state_at( const Board::Position& pos ) const
            -> Cell_state::Enum
        { return m_board.at( pos ).state(); }

        struct Stride{ int dx; int dy; };
        static void move( Position& pos, const Stride& stride )
        {
            pos.x += stride.dx;  pos.y += stride.dy;
        }

        auto winner_from( const Position& start_pos, const Stride& stride ) const
            -> Cell_state::Enum
        {
            const Cell_state::Enum first = board_state_at( start_pos );
            if( first == empty ) {
                return empty;
            }
            Position pos = start_pos;
            for( int i = 1; i <= Board::size - 1; ++i ) {
                move( pos, stride );
                if( board_state_at( pos ) != first ) {
                    return empty;
                }
            }
            return first;
        }

    public:
        auto board() const      -> const Board&     { return m_board; }
        auto n_moves() const    -> int              { return m_n_moves; }
        auto outcome() const    -> Outcome::Enum    { return m_outcome; }

        auto state_for_move() const
            -> Cell_state::Enum
        { return (is_even( m_n_moves )? cross : circle); }

        auto is_valid_move( const Board::Position& pos ) const
            -> bool
        { return m_outcome == Outcome::ongoing and board_state_at( pos ) != empty; }

        auto winner() const
            -> Cell_state::Enum
        {
            for( int y = 0; y < Board::size; ++y ) {
                const auto win_state = winner_from( {0, y}, Stride{1, 0} );
                if( win_state != empty ) { return win_state; }
            }
            for( int x = 0; x < Board::size; ++x ) {
                const auto win_state = winner_from( {x, 0}, Stride{0, 1} );
                if( win_state != empty ) { return win_state; }
            }
            for( int i = 0; i < Board::size; ++i ) {
                const auto win_state = winner_from( {0, 0}, Stride{1, 1} );
                if( win_state != empty ) { return win_state; }
            }
            for( int i = 0; i < Board::size; ++i ) {
                const auto win_state = winner_from( {Board::max_index, 0}, Stride{-1, 1} );
                if( win_state != empty ) { return win_state; }
            }
            return empty;
        }

        void move( const Board::Position& pos )
        {
            assert( is_valid_move( pos ) );

            const Cell_state::Enum state_played = state_for_move();
            m_board.at( pos ) = state_played;
            ++m_n_moves;

            const Cell_state::Enum winner_state = winner();
            assert( winner_state == empty or winner_state == state_played );
            if( winner_state == cross ) {
                m_outcome = Outcome::crosses_won;
            } else if( winner_state == circle ) {
                m_outcome = Outcome::circles_won;
            } else if( m_n_moves == max_moves ) {
                m_outcome = Outcome::all_cells_played;
            }
        }
    };
}  // namespace ttt
