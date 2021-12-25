# Windows GUI-stuff in C++: a tutorial.

## Part 3 – A Tic-Tac-Toe game in a general window.

In the previous part we saw that an application’s icon is provided via data embedded in the executable, called a resource. Now we’ll additionally use a resource called a **dialog template**, that describes the contents of a simple window. As a concrete example we’ll use that for a simple Tic-Tac-Toe (tree in a row) game in a general window.

That window contains two text areas and nine flat buttons, specified by the dialog template:

![The Tic-Tac-Toe game window](part-03/images/sshot-1.the-final-window.png)

The game logic is just a little detail at the end, though. Mainly the five (!) versions of the program exemplify general Windows programming techniques and issues, a kind of C style **event based programming**. The parts of the Windows API used here are all designed for use from C, and we’ll use this API more or less directly with C++ as just a “better C”.


<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [3.1. Creating and running a dialog template based general window.](#31-creating-and-running-a-dialog-template-based-general-window)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---

### 3.1. A general window based on a dialog template.

In order to keep things simple now, there will be a host of issues that are deferred, problems that will need fixing in later versions. So version 1 is an incomplete and quite imperfect window. It’s even without a custom window icon!

![The v1 window](part-03/images/sshot-2.main-window-v1.png)

Only two absolutely crucial problems are addressed in this version:

* The default behavior is, quite unreasonably, that any attempt to close the window has *no effect*.
* Windows 11 often fails to [**activate**](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-features#active-window) a new window, so that in many situations running a program has *no visible effect* where you’re looking (though an icon may appear in the taskbar).

For now and until version 4 we’ll deal with the activation problem by just specifying in the dialog template that the window should be in **topmost** mode, where it always is very visible above all normal mode window. The problem with that is that the window also appears above existing topmost windows, such as on my machine the on-screen analog clock. Version 4 will turn off topmost mode after the window has been created, leaving the window above other normal mode windows but allowing it to slip beneath topmost windows that you really want to see.



The window closing problem requires a programmatic solution. 


---
asdasdasd
The general


A Windows **dialog** is usually a window that pops up on top of the one the user works in, where the user must provide some information or at least cancel the dialog before the main work can be resumed. A message box is a roughly minimal dialog, and an application’s “about box” is another example. Part of the support for such dialogs is the possibility of creating one very simply via the API function **`DialogBox`**, with the window contents specified by a **dialog template** resource such as

~~~ rc
IDD_MAIN_WINDOW DIALOGEX 0, 0, 344, 132
STYLE DS_SETFONT | DS_CENTER | WS_CAPTION | WS_SYSMENU
EXSTYLE WS_EX_OVERLAPPEDWINDOW | WS_EX_TOPMOST
CAPTION "Tic-Tac-Toe (mostly random)"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    LTEXT           "Click a place on the board,  or type its digit 1 through 9.", 0,
                    7, 7, 313, 11, SS_NOPREFIX
    PUSHBUTTON      "7", BOARD_BUTTON_BASE + 7, 5, 24, 36, 32, BS_FLAT
    PUSHBUTTON      "8", BOARD_BUTTON_BASE + 8, 43, 24, 36, 32, BS_FLAT
    PUSHBUTTON      "9", BOARD_BUTTON_BASE + 9, 81, 24, 36, 32, BS_FLAT
    PUSHBUTTON      "4", BOARD_BUTTON_BASE + 4, 5, 58, 36, 32, BS_FLAT
    PUSHBUTTON      "5", BOARD_BUTTON_BASE + 5, 43, 58, 36, 32, BS_FLAT
    PUSHBUTTON      "6", BOARD_BUTTON_BASE + 6, 81, 58, 36, 32, BS_FLAT
    PUSHBUTTON      "1", BOARD_BUTTON_BASE + 1, 5, 93, 36, 32, BS_FLAT
    PUSHBUTTON      "2", BOARD_BUTTON_BASE + 2, 43, 93, 36, 32, BS_FLAT
    PUSHBUTTON      "3", BOARD_BUTTON_BASE + 3, 81, 93, 36, 32, BS_FLAT
    LTEXT           "<The rules should be displayed here>", IDC_RULES_DISPLAY,
                    123, 25, 217, 96, SS_NOPREFIX
END
~~~

The above is not the complete resource file, just the dialog specification part. Here `IDD_MAIN_WINDOW`, `BOARD_BUTTON_BASE` and `IDC_RULES_DISPLAY` are ordinary C++ macros defined as arbitrary small integers. I defined them as respectively 101, 1000 and 102. They’re macros because both the resource compiler and the C++ compiler understand macros, so that they can connect the C++ code to the resource (via various API functions). The other uppercase words are a mixture of dialog resource syntax words and macros defined by `<windows.h>`.

I guess you can see why it’s a good idea to at least initially *generate* this via some dialog design tool, and about the only usable one now is the **resource editor** in Visual Studio.

After generating most of that with Visual Studio’s dialog editor I copied and tidied up the result, including using symbolic expressions like `BOARD_BUTTON_BASE + 7` for the button id’s. Unfortunately VS doesn’t support that. And unfortunately VS also has some other limitations, such as generating code that the resource compiler flags as error, and such as refusing to accept text longer than 256 characters, which is the reason for the “\<The rules should be displayed here\>” — we’ll fix that programmatically in version 2.

Anyway, the definition yields a dialog like this:

![The v1 window](part-03/images/sshot-2.main-window-v1.png)

This window is quite a bit wider than 344 pixels, and quite a bit higher than 132 pixels. That’s because the units in the dialog specification are **dialog units**. A Windows dialog unit is proportional to the font size, much like the “em” unit in CSS, and was supposed to make dialogs automatically scaleable.

---

Unfortunately this dialog is *on top* of every other window. That’s because the dialog definition specifies the `WS_EX_TOPMOST` style. It’s a hack to work around Windows 11’s tendency to not activate a new window so that it can be partially or fully hidden behind other windows. We’ll turn off that restrictive **topmost window** mode in version 4, but until then it’s a way to guarantee that you’ll see the window when you try this, i.e. it’s a way to avoid believing erroneously that nothing happens when you run the program (the fix in version 4 still lets you see the window, though it doesn’t activate).


asdasd


--- 

Versions 1 is minimal code to get the window up and running, based on the dialog template. Essentially this uses Windows’s `DialogBox` function to launch the window, and provides a callback function to handle window closing, customizing that. For unfortunately the default functionality doesn’t let the user close the window…

Version 2 mainly adds a custom window icon (upper left in the window), and the rules text in the text display to the right. The window icon needs to be added programmatically because dialog templates don’t support specifying an icon. And the rules text “needs” to be set programmatically because the Visual Studio dialog template editor refuses to accept a text larger than 256 characters, which is one of a number of ways that even Microsoft’s flagship IDE fails to fully support dialog templates.

Other dialog template editors have even worse limitations. The tool support is lacking because the simple windows that dialog templates once made simpler to create, relative to the technology of that time, can now be done in much much simpler ways e.g. as web applications, or in Windows via C# in .NET. So, dialog templates are an almost archaic technology, but they serve well for introducing a number of Windows programming techniques and concepts.

Version 3 is a refactoring introducing the also archaic C language oriented *message cracker* macros from `<windowsx.h>`; a change that doesn’t affect the functionality, just improving the maintainability.

Version 4 fixes some gross imperfection issues that are due to the evolving nature of Windows, including standard GUI font (always a problem in Windows), getting a new window on top (a problem introduced with Windows 11), and modern look and feel (a problem created in Windows Vista).

Finally, in version 5 the game logic is added, plus a few UI features for a better user experience.
