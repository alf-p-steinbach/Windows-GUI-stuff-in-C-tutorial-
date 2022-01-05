# Windows GUI-stuff in C++: a tutorial.

## Part 4 – UTF-8 as the one and only text encoding.

In the previous version there was a character encoding problem, that a right single quote mark in a C++ string literal, would not necessarily be displayed correctly.

The problem happened because the controls in a window assume that `char` based text is encoded with the process’ **Windows ANSI** encoding, the encoding specified by the `GetACP` function, which is usually some variant of Windows ANSI Western, whereas the encoding for a C++ literal typically is UTF-8 — at least for experienced programmers.

So, in this part we’ll change everything to UTF-8 encoding. Which involves telling the tools that they should assume and produce UTF-8. This gets rid of the right single quote problem, and also enables use of some fancy Unicode characters. 😃

![Unicode symbols in the main window.](part-04/images/sshot-1.unicode-symbols-in-ttt.png)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [4.1. Some background info.](#41-some-background-info)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---

### 4.1. Some background info.

TLDR: by using UTF-8 we are at the leading edge of Windows desktop software development where not all the wrinkles have been ironed out yet, and we only support Windows versions since June 2019.

<p align="center">* * *</p>

The [**Unicode** character set](https://home.unicode.org/basic-info/faq/) is the basis of a number of possible text encodings, where the most popular in Windows are `char`-based UTF-8 and `wchar_t`-based UTF-16.

UTF-16 encoded text is a sequence of 16-bit encoding values which in Windows C and C++ programming are of type **`wchar_t`**. The “`w`” in `wchar_t` stands for **wide text**, text represented with “`wide`” encoding values. In C and C++ you can express wide text literals with prefix **`L`**, e.g. `L"Hello!"`.

But note: in Linux and other Unixes `wchar_t` is in general 32 bits, using UTF-32 encoding. So, while `L"Hello!"` is a sequence of 7 16-bit `wchar_t` values in Windows, it’s a sequence of 7 32-bit `wchar_t` values in Linux. Plus, some Unicode code points that can be represented with a single 32-bit `wchar_t` value in Linux, require two 16-bit `wchar_t` values in Windows, an UTF-16 **surrogate pair**, so that a wide literal doesn’t even always produce the same sequence of values in Linux and Windows.

When we used `MessageBox` like

~~~cpp
MessageBox( 0, "Some text in the box", "A title", MB_SETFOREGROUND );
~~~

… we actually used a `<windows.h>` macro that was defined as `MessageBoxA`, the message box function that takes `char` based string arguments:

~~~cpp
MessageBoxA( 0, "Some text in the box", "A title", MB_SETFOREGROUND );
~~~

This function just translates the text to `wchar_t` based UTF-16 and calls `MessageBoxW` (suffix “A” for “ANSI” and suffix “W” for “wide”), as if one had coded

~~~cpp
MessageBoxW( 0, L"Some text in the box", L"A title", MB_SETFOREGROUND );
~~~

That is, the Windows core functionality is based on UTF-16 encoding, but `<windows.h>` defaults to offering wrapper functions that convert `char` based text to `wchar_t` based UTF-16, and vice versa. This wrapper layer originated as a compatibility layer for old `char` based applications. The intent was, in the early and mid 1990’s, that new Windows programs should use wide text, e.g. `MessageBoxW`.

And up [until June 2019](https://docs.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8) one had to use UTF-16 encoded wide text in that way in any serious Windows desktop programming, because that was the only Unicode encoding supported by the API functions. One then defined the macro symbol **`UNICODE`** before including `<windows.h>`, so that `<windows.h>` would define the function-name macros to map to the “W” functions. For example, with `UNICODE` defined before `<windows.h>` is included, it defines `MessageBox` as `MessageBoxW`.

However, from June 2019 Windows supports using UTF-8 as a process’ `char`-based encoding, i.e. the process’ ANSI encoding, the one specified by [`GetACP()`](https://docs.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-getacp). Also, from this point on a Windows locale can have UTF-8 as its text encoding. And a C or C++ locale can therefore also have UTF-8 encoding provided that the C or C++ standard library implementation supports that.

With this the `char` based wrapper layer, the “A” suffix functions like `MessageBoxA`, has been *re-purposed to support the future* instead of supporting the past, just now with the “A” very much a misleading misnomer. This is  much like how C++ `auto` was re-purposed to support the future instead of supporting the past, just now with the “auto” very much a misleading misnomer… Well, I guess there must be an example also in the human body, maybe.

Anyway, as of early 2022 the UTF-8 support is far from complete.

However, together with earlier introduced UTF-8 support in the Visual C++ compiler and other Microsoft tools the June 2019 support in Windows *enables* use of UTF-8 throughout. This avoids text encoding problems such as for the right single quote in the previous version of our Tic-Tac-Toe game. And it enables use of special Unicode characters provided that the font one uses, supports them.


| ← previous |  up ↑ | next → |
|:----|:----:|---:|
| [3. A Tic-Tac-Toe game in a dialog window.](part-03.md) | [Contents](index.md)  | &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ? |
