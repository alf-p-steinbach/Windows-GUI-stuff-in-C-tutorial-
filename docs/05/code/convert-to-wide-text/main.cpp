# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <stdlib.h>         // EXIT_FAILURE
#include <string>           // std::wstring;
#include <string_view>      // std::string_view
using   std::wstring, std::string_view;

auto main() -> int
{
    constexpr auto& text        = "Every 日本国 кошка loves\nNorwegian blåbærsyltetøy!";
    constexpr auto  text_length = static_cast<int>( string_view( text ).length() );
    
    constexpr auto buffer_size = text_length;
    auto wide_text = wstring( buffer_size, L'\0' );
    const auto flags = DWORD( 0 );
    const int n_wide_values = MultiByteToWideChar(
        CP_UTF8, flags, text, text_length, &wide_text[0], buffer_size
        );
    if( n_wide_values == 0 ) { return EXIT_FAILURE; }
    wide_text.resize( n_wide_values );

    MessageBoxW( 0, wide_text.c_str(), L"UTF-16 text:", MB_ICONINFORMATION | MB_SETFOREGROUND );
}
