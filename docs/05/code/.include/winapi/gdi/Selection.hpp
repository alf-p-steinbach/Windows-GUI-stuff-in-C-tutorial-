#error
    class Selection: No_copying
    {
        HDC         m_dc;
        HGDIOBJ     m_original_object;
        
    public:
        ~Selection() { ::SelectObject( m_dc, m_original_object ); }
        
        Selection( const HDC dc, const HGDIOBJ object ):
            m_dc( dc ),
            m_original_object( ::SelectObject( dc, object ) )
        {}
    };
