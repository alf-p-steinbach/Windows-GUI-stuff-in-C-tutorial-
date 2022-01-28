# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
#include <winapi/gdi-Bitmap.hpp>            // winapi::gdi::Bitmap
#include <winapi/gui-util.hpp>              // winapi::gui::std_gui_font
#include <winapi/ole2.hpp>                  // Ole_library_usage

#include <shlwapi.h>        // SHCreateStreamOnFileEx
#include <stdio.h>          // fprintf
#include <stdlib.h>         // EXIT_...
#include <olectl.h>         // OleCreatePictureIndirect
#include <ocidl.h>          // IPicture

#include <stdexcept>        // std::exception&
#include <string>           // std::string
#include <string_view>      // std::(string_view, wstring_view)

namespace cu = cpp::util;
namespace ole2 = winapi::ole2;
namespace gdi = winapi::gdi;
using   cu::hopefully, cu::fail, cu::No_copying, cu::int_size, cu::Const_;
using   gdi::Bitmap;
using   std::string,
        std::string_view, std::wstring_view;

void display_graphics_on( const HDC canvas )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  area        = RECT{ 0, 0, 400, 400 };
    
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );   // `0` works for me, but should perhaps be the DC brush.

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
}


class Dc: No_copying
{
    HDC     m_handle;
       
protected:
    inline virtual ~Dc() = 0;

    Dc( const HDC handle ): m_handle( handle )
    {
        hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
    }

public:    
    auto handle() const -> HDC { return m_handle; }
};
inline Dc::~Dc() {}


class Screen_dc: public Dc
{
    static constexpr auto no_window = HWND( 0 );

public:        
    ~Screen_dc() { ReleaseDC( no_window, handle() ); }
    Screen_dc(): Dc( GetDC( no_window ) ) {}
};


class Memory_dc: public Dc
{
public:
    ~Memory_dc() { DeleteDC( handle() ); }
    Memory_dc(): Dc( CreateCompatibleDC( {} ) ) {}
};

class Bitmap_dc: public Memory_dc
{
    Bitmap  m_bitmap;

public:
    Bitmap_dc( const int width, const int height ): Memory_dc(), m_bitmap( width, height )
    {
        SelectObject( handle(), m_bitmap.handle() );
    }
};


class B_string: No_copying
{
    BSTR    m_pointer;
    
public:
    ~B_string() { SysFreeString( m_pointer ); }

    B_string( const wstring_view& ws ):
        m_pointer( SysAllocStringLen( ws.data(), int_size( ws ) ) )
    {
        hopefully( m_pointer ) or CPPUTIL_FAIL( "SysAllocStringLen failed" );
    }
    
    B_string( const string_view& s ): B_string( winapi::to_utf16( s ) ) {}
    
    operator BSTR() const { return m_pointer; }     // Intentionally ignores C++20 `<=>`.
};


void save_to( const string_view& file_path, Const_<IPictureDisp*> p_picture )
{
    const auto bstr_file_path = B_string( file_path );
    const HRESULT hr = OleSavePictureFile( p_picture, bstr_file_path );
    hopefully( SUCCEEDED( hr ) ) or CPPUTIL_FAIL( "OleSavePictureFile failed" );
}

template< class Interface >
class Com_ptr_: No_copying
{
    Interface*  m_ptr;
    
public:
    ~Com_ptr_() { m_ptr->Release(); }
    Com_ptr_( Const_<Interface*> ptr ): m_ptr( ptr ) {}
    auto raw() const -> Interface* { return m_ptr; }
};

auto ole_picture_from( const HBITMAP bitmap )
    -> Com_ptr_<IPictureDisp>
{
    PICTDESC params = { sizeof( PICTDESC ) };
    params.picType      = PICTYPE_BITMAP;
    params.bmp.hbitmap  = bitmap;

    IPictureDisp* p_picture_disp;
    const HRESULT hr = OleCreatePictureIndirect(
        &params, __uuidof( IPictureDisp ), false, reinterpret_cast<void**>( &p_picture_disp )
        );
    hopefully( SUCCEEDED( hr ) ) or CPPUTIL_FAIL( "OleCreatePictureIndirect failed" );
    return p_picture_disp;
}

void save_to( const string_view& file_path, const HBITMAP bitmap )
{
    save_to( file_path, ole_picture_from( bitmap ).raw() );
}

auto bitmap_of( const HDC dc )
    -> HBITMAP
{ return static_cast<HBITMAP>( GetCurrentObject( dc, OBJ_BITMAP ) ); }
    
void init( const HDC canvas )
{
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SetBkMode( canvas, TRANSPARENT );       // Don't fill in the background of text, please.
    SelectObject( canvas, winapi::gui::std_gui_font.handle );
}

void display_graphics()
{
    const int width     = 400;
    const int height    = 400;
    const auto bitmap_dc = Bitmap_dc( width, height );
    init( bitmap_dc.handle() );
    display_graphics_on( bitmap_dc.handle() );
    //BitBlt( Screen_dc().handle, 15, 15, width, height, bitmap_dc.handle, 0, 0, SRCCOPY );
    save_to( "generated-image.bmp", bitmap_of( bitmap_dc.handle() ) );
}

auto main( int, char** args ) -> int
{
    using   std::exception;
    try {
        const ole2::Library_usage _;
        display_graphics();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
        const auto as_errorbox  = MB_ICONERROR | MB_SYSTEMMODAL;
        const auto title        = string() + args[0] + " failed:";
        const auto text         = string() + "Because:\n" + x.what();
        MessageBox( 0, text.c_str(), title.c_str(), as_errorbox );
    }
    return EXIT_FAILURE;
}
