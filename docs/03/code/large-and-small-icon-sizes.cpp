// Using the following Windows libraries: user32, shell32, comctl32, ole32 and gdi32.
#include <windows.h>        // GetSystemMetrics
#undef small                // (!)

#include <commctrl.h>       // ImageList_GetIconSize
#include <objbase.h>        // CoInitialize    
#include <shellapi.h>       // SHGetFileInfo

#include <stdlib.h>         // EXIT_...
#include <stdio.h>          // printf

#include <functional>
#include <stdexcept>
#include <string>
using std::function, std::exception, std::runtime_error, std::string, std::to_string;

auto fail( const string& message ) -> bool { throw runtime_error( message ); }

namespace icon_sizes{
    enum Enum{ small = ICON_SMALL, large = ICON_BIG };   // WM_SETICON values.
}  // namespace icon_sizes

auto system_icon_pixelsize( const icon_sizes::Enum size )
    -> int
{ return GetSystemMetrics( size == icon_sizes::small? SM_CXSMICON : SM_CXICON ); }

auto shell_icon_pixelsize( const icon_sizes::Enum size )
    -> int
{
    SHFILEINFO sh_info = {};
    const auto gfi_iconsize =
        (size == icon_sizes::small? SHGFI_SMALLICON : SHGFI_LARGEICON);
    const DWORD_PTR gfi_result = SHGetFileInfo(
        "",     // Filename.
        {},     // File attributes.
        &sh_info, sizeof( sh_info ),
        SHGFI_SYSICONINDEX | SHGFI_SHELLICONSIZE | gfi_iconsize
        );
    (gfi_result != 0) or fail( "SHGetFileInfo failed" );
    
    const auto image_list = reinterpret_cast<HIMAGELIST>( gfi_result );
    int w = -1; int h = -1;
    ImageList_GetIconSize( image_list, &w, &h )
        or fail( "ImageList_GetIconSize failed" );
    const int pixel_size = w;

    return pixel_size;
}

void cpp_main()
{
    using namespace icon_sizes;
    const int   small_system_size   = system_icon_pixelsize( small );
    const int   large_system_size   = system_icon_pixelsize( large );

    printf( "System small and large icon sizes in pixels: %dx%d and %dx%d.\n",
        small_system_size, small_system_size, large_system_size, large_system_size
        );

    const int   small_shell_size    = shell_icon_pixelsize( small );
    const int   large_shell_size    = shell_icon_pixelsize( large );

    printf( "Shell  small and large icon sizes in pixels: %dx%d and %dx%d.\n",
        small_shell_size, small_shell_size, large_shell_size, large_shell_size
        );
}

auto main() -> int
{
    try {
        CoInitialize( nullptr );    // Needed for `SHGetFileInfo` call.
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
    }
    return EXIT_FAILURE;
}
