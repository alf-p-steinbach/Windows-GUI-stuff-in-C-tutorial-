#include <stdio.h>
#include <stdlib.h>

auto main() -> int
{
    system( "title A console program" );
    const auto& text =
        "This is a console window.\n"
        "\n"
        "A console window presents pure text. There can be no general graphics here.\n"
        "However, it's possible to do simple line drawing with characters.";
    printf( "%s", text );
    getchar();
}
