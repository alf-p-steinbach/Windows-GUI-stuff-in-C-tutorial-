#include <windows.h>

auto main() -> int
{
    const auto& text = L""
        "This is a general graphical window.\n"
        "\n"
        "Well actually it’s a message box. But it can contain general graphics"
        " such as the icon on the left, and interactive GUI elements such as the"
        " OK button.\n";
    
    MessageBoxW( 0, text, L"A GUI program", MB_ICONINFORMATION | MB_SETFOREGROUND );
}
