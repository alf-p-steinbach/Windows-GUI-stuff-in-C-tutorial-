#include <windows.h>

auto main() -> int
{
    const auto& title   = "Hello, world!";
    const auto& text    = "Click the OK button to quit, please:";

    MessageBox( 0, text, title, MB_SETFOREGROUND );
}
