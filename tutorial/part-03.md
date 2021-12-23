# Windows GUI-stuff in C++: a tutorial.

## Part 3 – A Tic-Tac-Toe game in a general window.

In the previous part we saw that an application’s icon is provided via data embedded in the executable, called a resource. Now we’ll additionally use a resource called a **dialog template**, that describes the contents of a simple window. As a concrete example we’ll use that for a simple Tic-Tac-Toe (tree in a row) game in a general window.

That window contains two text areas and nine flat buttons, specified by the dialog template:

![The Tic-Tac-Toe game window](part-03/images/sshot-1.the-final-window.png)

The game itself is just a little detail at the end, though.

Versions 1 is minimal code to get the window up and running, based on the dialog template. Essentially this uses Windows’s `DialogBox` function to launch the window, and provides a callback function to handle window closing, customizing that. For unfortunately the default functionality doesn’t let the user close the window…

Version 2 mainly adds a custom window icon (upper left in the window), and the rules text in the text display to the right. The window icon needs to be added programmatically because dialog templates don’t support specifying an icon. And the rules text “needs” to be set programmatically because the Visual Studio dialog template editor refuses to accept a text larger than 256 characters, which is one of a number of ways that even Microsoft’s flagship IDE fails to fully support dialog templates.

Other dialog template editors have even worse limitations. The tool support is lacking because the simple windows that dialog templates once made simpler to create, relative to the technology of that time, can now be done in much much simpler ways e.g. as web applications, or in Windows via C# in .NET. So, dialog templates are an almost archaic technology, but they serve well for introducing a number of Windows programming techniques and concepts.

Version 3 is a refactoring introducing the also archaic C language oriented *message cracker* macros from `<windowsx.h>`; a change that doesn’t affect the functionality, just improving the maintainability.

Version 4 fixes some gross imperfection issues that are due to the evolving nature of Windows, including standard GUI font (always a problem in Windows), getting a new window on top (a problem introduced with Windows 11), and modern look and feel (a problem created in Windows Vista).

Finally, in version 5 the game logic is added, plus a few UI features for a better user experience.


<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [2.1. File icons.](#21-file-icons)
- [2.2. Icon files.](#22-icon-files)
- [2.3. Package the icon in a linker friendly binary file, via a resource script.](#23-package-the-icon-in-a-linker-friendly-binary-file-via-a-resource-script)
- [2.4. The C++ side: identify the resource via a *pseudo-pointer* and a *module handle*.](#24-the-c-side-identify-the-resource-via-a-pseudo-pointer-and-a-module-handle)
- [2.5 Build the executable with resources embedded.](#25-build-the-executable-with-resources-embedded)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---

### 3.1. File xxx.

