# Windows GUI-stuff in C++: a tutorial.

Desktop Windows programming starting from the ground up in C++17.

I haven’t decided how far to take this. I’ll add more as I have free time to fill, maybe once a week or two weeks. Free time include those times when I get too bored with other stuff.

*TLDR short contents:*
- [1. A Windows GUI “Hello, world!”.](01.md)
- [2. Use resources to provide an icon.](02.md)
- [3. A Tic-Tac-Toe game in a dialog window.](03.md)
- [4. UTF-8 as the one and only text encoding.](04.md)
- [5. GDI graphics: Basics.](05.md)

*Full contents:*

- [1. A Windows GUI “Hello, world!”.](01.md)
    - [1.1. Message boxes.](01.md#11-message-boxes)
    - [1.2. About modern C++ syntax.](01.md#12-about-modern-c-syntax)
    - [1.3. Windows subsystems.](01.md#13-windows-subsystems)
    - [1.4. Building for the GUI subsystem.](01.md#14-building-for-the-gui-subsystem)
    - [1.5. Find required libraries and headers.](01.md#15-find-required-libraries-and-headers)
    - [1.6. Run the program via commands and via mouse clicking.](01.md#16-run-the-program-via-commands-and-via-mouse-clicking)
    - [1.7. Varying behaviors of programs when you run them in Windows 11.](01.md#17-varying-behaviors-of-programs-when-you-run-them-in-windows-11)
- [2. Use resources to provide an icon.](02.md)
    - [2.1. File icons.](02.md#21-file-icons)
    - [2.2. Icon files.](02.md#22-icon-files)
    - [2.3. Package the icon in a linker friendly binary file, via a resource script.](02.md#23-package-the-icon-in-a-linker-friendly-binary-file-via-a-resource-script)
    - [2.4. The C++ side: identify the resource via a *pseudo-pointer* and a *module handle*.](02.md#24-the-c-side-identify-the-resource-via-a-pseudo-pointer-and-a-module-handle)
    - [2.5 Build the executable with resources embedded.](02.md#25-build-the-executable-with-resources-embedded)
- [3. A Tic-Tac-Toe game in a dialog window.](03.md)
    - [3.1. Create and run a general window based on a dialog template.](03.md#31-create-and-run-a-general-window-based-on-a-dialog-template)
    - [3.2. Add a window icon and the too longish text by sending window messages.](03.md#32-add-a-window-icon-and-the-too-longish-text-by-sending-window-messages)
    - [3.3. Factor out: `<windows.h>` inclusion; support machinery; window message cracking.](03.md#33-factor-out-windowsh-inclusion-support-machinery-window-message-cracking)
    - [3.4. Fix gross imperfections: standard font; window just on top; modern appearance.](03.md#34-fix-gross-imperfections-standard-font-window-just-on-top-modern-appearance)
    - [3.5. Add the game logic and user interaction.](03.md#35-add-the-game-logic-and-user-interaction)
- [4. UTF-8 as the one and only text encoding.](04.md)
    - [4.1. Some background on Unicode in Windows programming.](04.md#41-some-background-on-unicode-in-windows-programming)
    - [4.2. Specify UTF-8 as the process’ ANSI code page.](04.md#42-specify-utf-8-as-the-process-ansi-code-page)
    - [4.3. Specify UTF-8 as the “.rc” resource script code page.](04.md#43-specify-utf-8-as-the-rc-resource-script-code-page)
    - [4.4. Adapt the C++ source code to UTF-8.](04.md#44-adapt-the-c-source-code-to-utf-8)
    - [4.5. Build with UTF-8 encoding throughout.](04.md#45-build-with-utf-8-encoding-throughout)
- [5. GDI graphics: Basics.](05.md)
    - [5.1. Draw on the screen, via a device context.](05.md#51-draw-on-the-screen-via-a-device-context)
    - [5.2 Use “DC colors” to reduce the drawing code.](05.md#52-use-dc-colors-to-reduce-the-drawing-code)
    - [5.3. Present graphics in a window by handling `WM_PAINT`.](05.md#53-present-graphics-in-a-window-by-handling-wm_paint)
    - [5.4. Support a dynamic window sized ellipse via `WM_SIZE` + `InvalidateRect`.](05.md#54-support-a-dynamic-window-sized-ellipse-via-wm_size--invalidaterect)
    - [5.5. Avoid animation flicker by handling `WM_ERASEBKGND` and using double buffering.](05.md#55-avoid-animation-flicker-by-handling-wm_erasebkgnd-and-using-double-buffering)

Perhaps a useful link: [The tutorial’s Github project.](https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-)

Also, if you want to report a problem or possible improvement, check out the [issues page](https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/issues).
