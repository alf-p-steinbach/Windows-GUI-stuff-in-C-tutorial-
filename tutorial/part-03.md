# Windows GUI-stuff in C++: a tutorial.

## Part 3 – A Tic-Tac-Toe game in a general window.

In the previous part we saw that an application’s icon is provided via data embedded in the executable, called a resource. Now we’ll additionally use a resource called a **dialog template**, that describes the contents of a simple window. As a concrete example we’ll use that for a simple Tic-Tac-Toe (tree in a row) game in a general window.

That window contains two text areas and nine flat buttons, specified by the dialog template:

![The Tic-Tac-Toe game window](part-03/images/sshot-1.the-final-window.png)

The game logic is just a little detail at the end, though. Mainly the five (!) versions of the program exemplify general Windows programming techniques and issues, a kind of C style **event based programming**. The parts of the Windows API used here are all designed for use from C, and we’ll use this API more or less directly with C++ as just a “better C”.


<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [3.1. A general window based on a dialog template.](#31-a-general-window-based-on-a-dialog-template)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---

### 3.1. A general window based on a dialog template.

In order to keep things simple now, there will be a host of issues that are deferred, problems that will need fixing in later versions. So version 1 is an incomplete and quite imperfect window. It’s even without a custom window icon!

![The v1 window](part-03/images/sshot-2.main-window-v1.png)

Only two absolutely crucial problems are addressed in this version:

* The default behavior is, quite unreasonably, that any attempt to close the window has *no effect*.
* Windows 11 often fails to [**activate**](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-features#active-window) a new window, so that in many situations running a program has *no visible effect* where you’re looking (though an icon may appear in the taskbar).

For now and until version 4 we’ll deal with the activation problem by just specifying in the dialog template that the window should be in **topmost** mode, where it’s always very visible above all normal mode windows. The main problem with that is that the window also appears above existing topmost windows, such as on my machine above the on-screen analog clock. For that reason version 4 will turn off topmost mode after the window has been created.

To address the window no-close problem we need to programmatically override the default handling of a window close attempt. Normally in C++ one would do that by overriding some virtual function, preferably using the C++ `override` keyword, and indeed that’s the way to do it with some C++ GUI frameworks. However, here we’re dealing with a C oriented part of the Windows API, where it needs to be done in a C-ish way via a freestanding function:

~~~ cpp
auto CALLBACK message_handler(
    const HWND              window,
    const UINT              msg_id,
    const WPARAM            ,       // w_param
    const LPARAM                    // ell_param
    ) -> INT_PTR
{
    if( msg_id == WM_CLOSE ) {
        EndDialog( window, IDOK );  // Without this the window won't close.
        return true;
    }
    return false;       // Didn't process the message, want default.
}
~~~

This function is specified in the call to **`DialogBox`** that creates and runs the window,

~~~ cpp
DialogBox( ⋯, message_handler );
~~~

The `DialogBox` call returns when the window is closed, i.e. it’s a blocking call. In the meantime the internal Windows code that runs the window calls the specified function each time something happens that application code can customize or react to. I.e. this is a **callback** function.

A callback for a dialog, in Windows parlance a “**dialog proc**”, can serve many dialog windows. Which window a call of the callback is about is specified by the `window` argument, somewhat akin to the `this` pointer in C++ programming. And exactly what happened or is about to happen is specied by the `msg_id` argument, a window **message**. With modern terminology it’s called an **event id**. The modern event terminology is however rarely used in Windows API-level window programming. Both Microsoft Windows and the Apple Macintosh GUI were created based on ideas from the research at Xerox PARC, and much of the Xerox PARC terminology, in particular [Smalltalk “message”](https://en.wikipedia.org/wiki/Smalltalk#Messages), was adopted.

The **`WM_CLOSE`** message (event id) is received when the user has attempted to close the window, e.g. by clicking the ✖ close button or via the `Alt`+`F4` keyboard shortcut. The default is that nothing happens, but our override/handler now calls `EndDialog`. Calling **`EndDialog`** is the proper way to close a window that was created via `DialogBox`.

The argument to `EndDialog`, here `IDOK`, specifies the value that `DialogBox` should return. `IDOK`, `IDCANCEL`, `IDYES` etc. are the id numbers of the possible buttons in a message box, and they serve as [standard return values from dialog windows](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox#return-value). It’s just an `int` return value that you can use for whatever.

Note about [the documentation of `DialogBox`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxa): it currently states that `DialogBoxA` is a macro. But `DialogBoxA` is a function. It’s `DialogBox` that is a macro, that by default resolves to `DialogBoxA`. Microsoft’s documentation used to be of less severe ungoodness. It’s worth keeping in mind that it needs to be read with a lot of understanding and sometimes *testing*, and an assumption that those technical writers that maintain it, don’t know the first thing about programming (as an example, as of late 2021 the documentation is still [full of invalid-in-C++ `void main`](https://www.google.com/search?q=%22void+main%22+site%3Ahttps%3A%2F%2Fdocs.microsoft.com%2Fen-us%2Fwindows%2Fwin32%2Fapi) beginners’ first thing mistakes).

Full C++ code:

[*part-03/code/tic-tac-toe/v1/main.cpp*](part-03/code/tic-tac-toe/v1/main.cpp)
~~~ cpp
// v1 - Roughly minimal code to display a window based on a dialog template resource.

#include <windows.h>
#include "resources.h"

auto CALLBACK message_handler(
    const HWND              window,
    const UINT              msg_id,
    const WPARAM            ,       // w_param
    const LPARAM                    // ell_param
    ) -> INT_PTR
{
    if( msg_id == WM_CLOSE ) {
        EndDialog( window, IDOK );  // Without this the window won't close.
        return true;
    }
    return false;       // Didn't process the message, want default.
}

auto main() -> int
{
    using C_str = const char*;
    const HINSTANCE this_executable     = GetModuleHandle( nullptr );
    const C_str     resource_id_as_ptr  = MAKEINTRESOURCE( IDD_MAIN_WINDOW );

    DialogBox( this_executable, resource_id_as_ptr, HWND(), message_handler );
}
~~~

Here the `"resources.h"` file just specifies the `IDD_MAIN_WINDOW` macro that resolves to a number that — together with a specification of which executable the resource is in — identifies the dialog template resource. It’s a macro because the resource compiler understands C macros but not C or C++ typed constants. In essence the C and C++ preprocessor is the glue, a common shared little language, that connects the C++ code with the resource script.

[*part-03/code/tic-tac-toe/v1/resources.h*](part-03/code/tic-tac-toe/v1/resources.h)
~~~ cpp
#pragma once

#define IDC_STATIC                      -1

#define IDD_MAIN_WINDOW                 101
#define IDC_RULES_DISPLAY               102
#define IDS_RULES                       103
#define IDI_APP                         104

#define BOARD_BUTTON_BASE               1000
~~~

asd

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
