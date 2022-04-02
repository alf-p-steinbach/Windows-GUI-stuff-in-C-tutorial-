# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <functional>       // std::function
#include <utility>          // std::move
#include <vector>           // std::vector

namespace c_curve {
    using std::function, std::move, std::vector;
    
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
        Point       m_current_position;
        Point       m_step;
        Callback    m_output_curve_point;

    public:
        Impl( Callback output_curve_point, const int step ):
            m_current_position{ 0, 0 },
            m_step{ 0, -step },
            m_output_curve_point( move( output_curve_point ) )
        {
            m_output_curve_point( m_current_position );
        }
        
        void generate( const int order )
        {
            if( order == 0 ) {
                m_current_position += m_step;
                m_output_curve_point( m_current_position );
            } else {
                generate( order - 1 );
                rotate_left( m_step );
                generate( order - 1 );
                rotate_right( m_step );
            }
        }
    };

    inline void generate( const int order, Callback output_curve_point, const int step = 4 )
    {
        Impl( move( output_curve_point ), step ).generate( order );
    }
    
    template< class Point_type >
    auto as_vector_of_( const int order, const int step = 4 )
        -> vector<Point_type>
    {
        vector<Point_type> points;
        points.reserve( 1 + (1 << order) );     // Just because this is C++ => efficiency.
        generate(
            order,
            [&]( const Point& pt ){ points.push_back( Point_type{ pt.x, pt.y } ); },
            step
            );
        return points;
    }
}  // namespace c_curve
