#include <stdio.h>

auto main() -> int
{
    using Byte = unsigned char;
    for( const Byte v: "Blåbærsyltetøy, nom!" ) {
        if( v ) { printf( "%d ", +v ); }
    }
    printf( "\n" );
}
