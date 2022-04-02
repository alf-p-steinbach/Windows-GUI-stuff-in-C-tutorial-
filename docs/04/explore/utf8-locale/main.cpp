#include <wrapped-winapi/windows-h.hpp>
#include <stdlib.h>         // EXIT_...
#include <iostream>
#include <iterator>         // size
#include <stdexcept>        // exception, runtime_error
#include <string>           // string
using namespace std;

auto fail( const string& s ) -> bool { throw runtime_error( s ); }
auto hopefully( const bool e ) -> bool { return e; }

template< class T >
auto int_size( const T& o ) -> int { return static_cast<int>( size( o ) ); }

const auto fail_char = char( 127 );     // Cannot be used with WideCharToMultiByte

auto to_utf8( const wstring_view& ws )
    -> string
{
    if( ws.empty() ) { return ""; }
    BOOL any_fail = false;
    const int buffer_size = WideCharToMultiByte(
        CP_UTF8, WC_NO_BEST_FIT_CHARS, ws.data(), int_size( ws ), nullptr, 0, nullptr, &any_fail
        );
    hopefully( buffer_size > 0 ) or fail( "WideCharToMultiByte #1" );
    auto result = string( buffer_size, '\0' );
    const int size = WideCharToMultiByte(
        CP_UTF8, WC_NO_BEST_FIT_CHARS, ws.data(), int_size( ws ), result.data(), int_size( result ),
        nullptr, &any_fail
        );
    hopefully( size > 0 ) or fail( "WideCharToMultiByte #2" );
    result.resize( size );
    return result;
}

auto wide_user_locale_spec()
    -> wstring
{
    auto spec = wstring( LOCALE_NAME_MAX_LENGTH, L'\0' );
    const int size = GetUserDefaultLocaleName( spec.data(), int_size( spec ) );
    hopefully( size > 0 ) or fail( "GetUserDefaultLocaleName" );
    spec.resize( size );
    return spec;
}

auto user_locale_spec()
    -> string
{ return to_utf8( wide_user_locale_spec() ); }

void cpp_main()
{
    cout << "Codepage: " << GetACP() << endl;
    cout << "Locale: “" << user_locale_spec() << "”" << endl;
}

auto main() -> int
{
    try {
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        cerr << "!" << x.what() << endl;
    }
    return EXIT_FAILURE;
}
