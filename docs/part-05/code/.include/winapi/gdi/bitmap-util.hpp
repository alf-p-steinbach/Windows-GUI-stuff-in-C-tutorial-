#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/ole/Library_usage.hpp>     // winapi::ole::Library_usage
#include <winapi/ole/picture-util.hpp>      // winapi::ole::save_to

#include <string_view>
#include <utility>

namespace winapi::gdi {
    namespace ole = winapi::ole;
    using std::string_view;

    inline void save_to( const string_view& file_path, const HBITMAP bitmap )
    {
        const ole::Library_usage _;     // RAII OleInitialize + OleUninitialize.
        ole::save_to( file_path, ole::picture_from( bitmap ).raw_ptr() );
        auto x = ole::picture_from( bitmap );
        auto y = ole::picture_from( bitmap );
        y = std::move( x );
        (void) y;
    }
}  // namespace winapi::gdi
