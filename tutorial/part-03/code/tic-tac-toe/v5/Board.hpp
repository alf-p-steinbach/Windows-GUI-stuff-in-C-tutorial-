#pragma once
#include <assert.h>
#include <array>
#include "calc.hpp"     // squared

namespace ttt {
    using std::array;
    
    class Board
    {
    public:
        enum{ size = 3, max_index = size - 1, n_cells = squared( size ) };

        class Cell
        {
        public:
            struct State
            {
                enum Enum: int{ empty, cross, circle };
                
                static constexpr auto opposite_of( const Enum v )
                    -> Enum
                { return array{ empty, circle, cross }[v]; }
            };
            
        private:
            State::Enum     m_state = State::empty;
            
        public:
            auto state() const -> State::Enum { return m_state; }

            void operator=( const State::Enum new_state )
            {
                assert( m_state == State::empty );
                assert( new_state != State::empty );
                m_state = new_state;
            }
        };

        struct Position
        {
            struct Rotation{ enum Enum{ r_0, r_90, r_180, r_270 }; };
            struct Flipping{ enum Enum{ none, horizontal }; };
            struct Transform{ Rotation rot; Flipping flip; };

            int     x;      // 0 .. 2 from left to right.
            int     y;      // 0 .. 2 from bottom to top.
        };

    private:
        array<Cell, n_cells>    m_cells     = {};
        
        static auto i_of( const Position& pos ) -> int { return pos.y*size + pos.x; }

    public:
        auto at( const Position& pos )
            -> Cell&
        { return m_cells[i_of( pos )]; }

        auto at( const Position& pos ) const
            -> const Cell&
        { return m_cells[i_of( pos )]; }
    };
}  // namespace ttt
