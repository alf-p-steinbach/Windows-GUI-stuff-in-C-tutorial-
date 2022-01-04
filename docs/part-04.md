# Windows GUI-stuff in C++: a tutorial.

## Part 4 – UTF-8 as the one and only text encoding.

In the previous version there was a character encoding problem, that a right single quote mark in a C++ string literal, would not necessarily be displayed correctly.

The problem happened because the controls in a window assume that `char` based text is encoded with the process’ **Windows ANSI** encoding, the encoding specified by the `GetACP` function, which is usually some variant of Windows ANSI Western, whereas the encoding for a C++ literal typically is UTF-8 — at least for experienced programmers.

So, in this part we’ll change everything to UTF-8 encoding. Which involves telling the tools that they should assume and produce UTF-8. This gets rid of the right single quote problem, and also enables use of fancy Unicode characters. 😃

(fig)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [3.1. Create and run a general window based on a dialog template.](#31-create-and-run-a-general-window-based-on-a-dialog-template)
- [3.2. Add a window icon and the too longish text by sending window messages.](#32-add-a-window-icon-and-the-too-longish-text-by-sending-window-messages)
- [3.3. Factor out: `<windows.h>` inclusion; support machinery; window message cracking.](#33-factor-out-windowsh-inclusion-support-machinery-window-message-cracking)
- [3.4. Fix gross imperfections: standard font; window just on top; modern appearance.](#34-fix-gross-imperfections-standard-font-window-just-on-top-modern-appearance)
- [3.5. Add the game logic and user interaction.](#35-add-the-game-logic-and-user-interaction)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---

### 4.1. Some background info.

The **Unicode** character set is the basis of a number of possible text encodings, where the most popular in Windows are `char`-based UTF-8 and `wchar_t`-based UTF-16.

In Windows **`wchar_t`** is 16 bits, whereas in Linux and other Unixes (which use UTF-32) in general it’s 32 bits. The C and C++ standards apparently reasonably insist that a single `wchar_t` should suffice for any code point in the character set, which requires 32 bits for a Unicode `wchar_t`. But both language standards are at odds with the roughly 30 years existing practice in the major desktop OS platform, i.e. the language standards are stuck in a childish academic “no no no!”-mode: no C or C++ compiler for Windows would be practically usable with 32-bits `wchar_t`, so no such compiler will be created.

The Windows core functionality is based on UTF-16 encoding. This is Windows’ variant of **wide text**, what the “`w`” in `wchar_t` is about. As an example, `L"Hello!"` is a wide string literal with 7 16-bit `wchar_t` values, corresponding to the **narrow text** literal `"Hello!"` with 7 (in Windows and on most systems:) 8-bit `char` values.

Up until June 2019 one had to use UTF-16 encoded wide text in any serious Windows desktop programming, because that was the only Unicode encoding supported by the API functions. However, from June 2019 Windows supports using UTF-8 as a process’ `char`-based encoding. Also, from this point on a Windows locale can have UTF-8 as its text encoding, and a C locale can therefore also have UTF-8 encoding provided the C standard library implementation supports that.

The UTF-8 support is far from complete, neither in Windows nor in the C and C++ standard libraries. For example, last I checked, in the Windows API the `DrawText` function assumed the system’s `char`-based encoding such as Windows ANSI Western, instead of the process’ `char`-based encoding that you’ve set to UTF-8, and for example, only the pure ASCII subset of UTF-8 is supported for input from consoles, precluding e.g. Norwegian input. And as an example of (still as of this writing) missing C++ support, outputting `char` based text to a C++ wide stream such as `wcout`, may use the system’ encoding.

However, together with earlier introduced UTF-8 support in the Visual C++ compiler and other Microsoft tools the June 2019 support in Windows *enables* use of UTF-8 throughout. This avoids encoding problems such as for the right single quote in the previous versions. And it enables use of special Unicode characters provided that the font one uses, supports them.



| ← previous |  up ↑ | next → |
|:----|:----:|---:|
| [3. A Tic-Tac-Toe game in a dialog window.](part-03.md) | [Contents](index.md)  | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ? |
