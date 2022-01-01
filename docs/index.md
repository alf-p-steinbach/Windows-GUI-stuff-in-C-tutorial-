# Windows GUI-stuff in C++: a tutorial.

Desktop Windows programming starting from the ground up in C++17.

I haven’t decided how far to take this. I’ll add more as I have free time to fill, maybe once a week or two weeks. Free time include those times when I get too bored with other stuff.

*Contents:*

- [1. A Windows GUI “Hello, world!”.](part-01.md)
    - [1.1. Message boxes.](part-01.md#11-message-boxes)
    - [1.2. About modern C++ syntax.](part-01.md#12-about-modern-c-syntax)
    - [1.3. Windows subsystems.](part-01.md#13-windows-subsystems)
    - [1.4. Building for the GUI subsystem.](part-01.md#14-building-for-the-gui-subsystem)
    - [1.5. Find required libraries and headers.](part-01.md#15-find-required-libraries-and-headers)
    - [1.6. Run the program via commands and via mouse clicking.](part-01.md#16-run-the-program-via-commands-and-via-mouse-clicking)
    - [1.7. Varying behaviors of programs when you run them in Windows 11.](part-01.md#17-varying-behaviors-of-programs-when-you-run-them-in-windows-11)
- [2. Use resources to provide an icon.](part-02.md)
    - [2.1. File icons.](part-02.md#21-file-icons)
    - [2.2. Icon files.](part-02.md#22-icon-files)
    - [2.3. Package the icon in a linker friendly binary file, via a resource script.](part-02.md#23-package-the-icon-in-a-linker-friendly-binary-file-via-a-resource-script)
    - [2.4. The C++ side: identify the resource via a *pseudo-pointer* and a *module handle*.](part-02.md#24-the-c-side-identify-the-resource-via-a-pseudo-pointer-and-a-module-handle)
    - [2.5 Build the executable with resources embedded.](part-02.md#25-build-the-executable-with-resources-embedded)
- [3. A Tic-Tac-Toe game in a dialog window.](part-03.md)
    - [3.1. Create and run a general window based on a dialog template.](#31-create-and-run-a-general-window-based-on-a-dialog-template)
    - [3.2. Add a window icon and the too longish text by sending window messages.](#32-add-a-window-icon-and-the-too-longish-text-by-sending-window-messages)
    - [3.3. Factor out: `<windows.h>` inclusion; support machinery; window message cracking.](#33-factor-out-windowsh-inclusion-support-machinery-window-message-cracking)
    - [3.4. Fix gross imperfections: standard font; window just on top; modern appearance.](#34-fix-gross-imperfections-standard-font-window-just-on-top-modern-appearance)
    - [3.5. Add the game logic and user interaction.](#35-add-the-game-logic-and-user-interaction)

Perhaps a useful link: [The tutorial’s Github project.](https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-)
