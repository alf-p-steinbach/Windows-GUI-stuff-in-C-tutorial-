#include <stdio.h>
#include <windows.h>

auto main() -> int
{
    printf( "For app: type = %d, id = %d.\n",
        static_cast<int>( reinterpret_cast<UINT_PTR>( RT_MANIFEST ) ),
        static_cast<int>( reinterpret_cast<UINT_PTR>( CREATEPROCESS_MANIFEST_RESOURCE_ID ) )
        );
}
