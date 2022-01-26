# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
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
using   cu::hopefully, cu::fail, cu::No_copying, cu::int_size, cu::Const_;
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

struct Bitmap_format
{ enum Enum{
    implied                 = 0,
    monochrome              = 1,
    palette_16_colors       = 4,
    palette_256_colors      = 8,
    rgb_compressed          = 16,
    rgb_24_bits             = 24,
    rgb_32_bits             = 32
}; };

struct Bitmap_handle_and_memory
{
    HBITMAP     handle;
    void*       p_bits;         // Owned by but cannot be obtained from the handle.
};

auto create_rgb32_bitmap(
    const int                   width,
    const int                   height
    ) -> Bitmap_handle_and_memory
{
    BITMAPINFO params  = {};
    BITMAPINFOHEADER& info = params.bmiHeader;
    info.biSize             = {sizeof( info )};
    info.biWidth            = width;
    info.biHeight           = height;
    info.biPlanes           = 1;
    info.biBitCount         = Bitmap_format::rgb_32_bits;
    info.biCompression      = BI_RGB;

    void* p_bits;
    const HBITMAP handle = CreateDIBSection(
        HDC(),              // Not needed because no DIB_PAL_COLORS palette.
        &params,
        DIB_RGB_COLORS,     // Irrelevant, but.
        &p_bits,
        HANDLE(),           // Section.
        0                   // Secotion offset.
        );
    hopefully( handle != 0 ) or CPPUTIL_FAIL( "CreateDibSection failed" );
    return Bitmap_handle_and_memory{ handle, p_bits };
}

class Bitmap
{
    HBITMAP     m_handle;

public:
    ~Bitmap() { DeleteObject( m_handle ); }
    
    Bitmap( const int width, const int height ):
        m_handle( create_rgb32_bitmap( width, height ).handle )
    {
        hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "CreateDIBSection failed" );
    }
    
    auto handle() const -> HBITMAP { return m_handle; }
};


constexpr auto no_window = HWND( 0 );

struct Screen_dc: No_copying
{
    HDC     handle;
        
    ~Screen_dc() { ReleaseDC( no_window, handle ); }

    Screen_dc():
        handle( GetDC( no_window ) )
    {
        hopefully( handle != 0 ) or CPPUTIL_FAIL( "GetDC() failed" );
    }
};

class Memory_dc: No_copying
{
    HDC     m_handle;
        
public:
    ~Memory_dc() { DeleteDC( m_handle ); }

    Memory_dc():
        m_handle( CreateCompatibleDC( {} ) )
    {
        hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "CreateCompatibleDC() failed" );
    }
    
    auto handle() const -> HDC { return m_handle; }
};

struct Bitmap_dc: Memory_dc
{
    ~Bitmap_dc()
    {
        DeleteObject( GetCurrentObject( handle(), OBJ_BITMAP ) );
    }

    Bitmap_dc( const int width, const int height ):
        Memory_dc()
    {
        const HBITMAP bmp = create_rgb32_bitmap( width, height ).handle;
        hopefully( bmp != 0 ) or CPPUTIL_FAIL( "CreateCompatibleBitmap failed" );
        SelectObject( handle(), bmp );
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
    
    B_string( const string_view& s ):
        B_string( winapi::to_utf16( s ) )
    {}
    
    operator BSTR() const { return m_pointer; }     // Intentionally ignores C++20 `<=>`.
};

void save_to( const string_view& file_path, Const_<IPictureDisp*> p_picture )
{
    const auto b_string = B_string( file_path );
    const HRESULT hr = OleSavePictureFile( p_picture, b_string );
    hopefully( SUCCEEDED( hr ) ) or CPPUTIL_FAIL( "OleSavePictureFile failed" );
}

template< class Interface >
class Com_ptr_: No_copying
{
    Interface*  m_ptr;
    
public:
    ~Com_ptr_() { m_ptr->Release(); }
    Com_ptr_( Const_<Interface*> ptr ): m_ptr( ptr ) {}
    auto raw_ptr() const -> Interface* { return m_ptr; }
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
    save_to( file_path, ole_picture_from( bitmap ).raw_ptr() );
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
