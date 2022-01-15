# // Source encoding: UTF-8 with BOM (p is a lowercase Greek "pi").

#include <functional>       // function
#include <utility>          // move

namespace dragon_curve {
    using std::function, std::move;
    
    struct Point { int x; int y; };
    void rotate_left( Point& pt )   { pt = {-pt.y, pt.x}; }
    void rotate_right( Point& pt )  { pt = {pt.y, -pt.x}; }

    void operator+=( Point& pt, const Point& d )
    {
        pt.x += d.x; pt.y += d.y;
    }

    using Callback = function<void( const Point& )>;

    class Impl
    {
        Point       m_current;
        Point       m_step;
        Callback    m_add_point;

    public:
        Impl( Callback add_point, const int step ):
            m_current{ 0, 0 },
            m_step{ 0, -step },
            m_add_point( move( add_point ) )
        {
            m_add_point( m_current );
        }
        
        void generate( const int order )
        {
            if( order == 0 ) {
                m_current += m_step;
                m_add_point( m_current );
            } else {
                generate( order - 1 );
                rotate_left( m_step );
                generate( order - 1 );
                rotate_right( m_step );
            }
        }
    };

    inline void generate( const int order, Callback add_point, const int step = 4 )
    {
        Impl( move( add_point ), step ).generate( order );
    }
}  // namespace dragon_curve
