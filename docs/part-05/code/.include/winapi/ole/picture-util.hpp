#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <cpp/util.hpp>                         // cpp::util::(success, Const_)
#include <winapi/com/failure-checking.hpp>      // winapi::com::failure_checking::operator>>
#include <winapi/ole/B_string.hpp>              // winapi::ole::B_string
#include <wrapped-winapi/ocidl-h.hpp>           // IPictureDisp
#include <wrapped-winapi/olectl-h.hpp>          // OleCreatePictureIndirect

namespace winapi::ole {
    namespace com   = winapi::com;
    namespace cu    = cpp::util;
    using   com::failure_checking::operator>>;
    using   cu::success, cu::Const_;

    inline auto picture_from( const HBITMAP bitmap )
        -> com::Ptr_<IPictureDisp>
    {
        PICTDESC params = { sizeof( PICTDESC ) };
        params.picType      = PICTYPE_BITMAP;
        params.bmp.hbitmap  = bitmap;

        IPictureDisp* p_picture_disp;
        OleCreatePictureIndirect(
            &params, __uuidof( IPictureDisp ), false, reinterpret_cast<void**>( &p_picture_disp )
            ) 
            >> success or CPPUTIL_FAIL( "OleCreatePictureIndirect failed" );
        return p_picture_disp;
    }

    inline void save_to( const string_view& file_path, Const_<IPictureDisp*> p_picture )
    {
        const auto bstr_file_path = B_string( file_path );
        OleSavePictureFile( p_picture, bstr_file_path )
            >> success or CPPUTIL_FAIL( "OleSavePictureFile failed" );
    }
}  // namespace winapi::ole
