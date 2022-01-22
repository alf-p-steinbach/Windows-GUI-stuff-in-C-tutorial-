# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
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
using   cu::hopefully, cu::fail, cu::No_copying, cu::int_size;
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

constexpr auto no_window = HWND( 0 );

struct Screen_dc: No_copying
{
    HDC     handle;
        
    ~Screen_dc() { ReleaseDC( no_window, handle ); }

    Screen_dc():
        handle( GetDC( no_window ) )
    {
        hopefully( handle != 0 )
            or fail( "GetDC() failed" );
    }
};

struct Memory_dc: No_copying
{
    HDC     handle;
        
    ~Memory_dc() { DeleteDC( handle ); }

    Memory_dc( const HDC properties ):
        handle( CreateCompatibleDC( properties ) )
    {
        hopefully( handle != 0 )
            or fail( "CreateCompatibleDC() failed" );
        SelectObject( handle, GetStockObject( DC_PEN ) );
        SelectObject( handle, GetStockObject( DC_BRUSH ) );
    }
};

struct Bitmap_dc: Memory_dc
{
    ~Bitmap_dc()
    {
        DeleteObject( GetCurrentObject( handle, OBJ_BITMAP ) );
    }

    Bitmap_dc( const HDC properties, const int width, const int height ):
        Memory_dc( properties )
    {
        const HBITMAP bmp = CreateCompatibleBitmap( properties, width, height );
        BITMAP info = {};
        GetObject( bmp, sizeof( info ), &info );
        fprintf( stderr, "planes = %ld, bits-per-pixel = %ld, width = %ld\n", info.bmPlanes, info.bmBitsPixel, info.bmWidth );
        hopefully( bmp != 0 )
            or fail( "CreateCompatibleBitmap failed" );
        SelectObject( handle, bmp );
    }
};

struct B_string: No_copying
{
    const BSTR  pointer;
    
    ~B_string() { SysFreeString( pointer ); }

    B_string( const wstring_view& ws ):
        pointer( SysAllocStringLen( ws.data(), int_size( ws ) ) )
    {
        hopefully( pointer != 0 ) or CPPUTIL_FAIL( "SysAllocStringLen failed" );
    }
    
    B_string( const string_view& s ):
        B_string( winapi::to_utf16( s ) )
    {}
};

void save_to( const string_view& file_path, IPictureDisp* p_picture )
{
    const auto b_string = B_string( file_path );
    const HRESULT hr = OleSavePictureFile( p_picture, b_string.pointer );
    hopefully( SUCCEEDED( hr ) ) or CPPUTIL_FAIL( "OleSavePictureFile failed" );
}

void display_graphics()
{
    const auto bitmap_dc = Bitmap_dc( Screen_dc().handle, 400, 400 );
    display_graphics_on( bitmap_dc.handle );
    BitBlt( Screen_dc().handle, 15, 15, 400, 400, bitmap_dc.handle, 0, 0, SRCCOPY );
    
    PICTDESC params = {sizeof( params )};
    params.picType = PICTYPE_BITMAP;
    params.bmp.hbitmap = static_cast<HBITMAP>( GetCurrentObject( bitmap_dc.handle, OBJ_BITMAP ) );
    IPictureDisp* p_picture_disp;
    const HRESULT hr2 = OleCreatePictureIndirect( &params, __uuidof( IPictureDisp ), false, (void**) &p_picture_disp );
    hopefully( SUCCEEDED( hr2 ) )
        or fail( "OleCreatePictureIndirect failed" );

    save_to( "generated-image.bmp", p_picture_disp );
    
    p_picture_disp->Release();
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
