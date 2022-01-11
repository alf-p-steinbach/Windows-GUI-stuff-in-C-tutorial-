# Windows GUI-stuff in C++: a tutorial.

## Part 5 – Primitive graphics: the GDI.

With last part’s discussion of how to use UTF-8 as the `char` based text encoding in both resource definitions and GUI code, we have a limited but useful framework for exploratory C++ Windows desktop programming.

We’ll use that to now explore the [**GDI**](https://en.wikipedia.org/wiki/Graphics_Device_Interface), Windows’ original *graphics device interface*, which supports basic graphics and text. GDI is simple and C-oriented, which is nice. On the other hand it’s slow and produces low quality graphics, in particular without anti-aliasing, which is a strong reason to later move on to the successor technologies [GDI+](https://en.wikipedia.org/wiki/Graphics_Device_Interface#Windows_XP) and [Direct 2D](https://en.wikipedia.org/wiki/Direct2D).

Unfortunately Windows doesn’t yet support custom UTF-8 based text presentation, i.e. for *drawing* text as graphics, as opposed to using controls to present text. We’ll work around that by writing wrappers around Windows’ wide text functions. Happily Windows does support conversion between UTF-8 (as well as a large number of other encodings) and UTF-16 via API functions such as [`MultiByteToWideChar`](https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar), in addition to, since Windows 10, providing the C API of the main Unicode library [**ICU**](https://docs.microsoft.com/en-us/windows/win32/intl/international-components-for-unicode--icu-).


[some figure]

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [4.1. Some background on Unicode in Windows programming.](#41-some-background-on-unicode-in-windows-programming)
- [4.2. Specify UTF-8 as the process’ ANSI code page.](#42-specify-utf-8-as-the-process-ansi-code-page)
- [4.3. Specify UTF-8 as the “.rc” resource script code page.](#43-specify-utf-8-as-the-rc-resource-script-code-page)
- [4.4. Adapt the C++ source code to UTF-8.](#44-adapt-the-c-source-code-to-utf-8)
- [4.5. Build with UTF-8 encoding throughout.](#45-build-with-utf-8-encoding-throughout)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---
### 5.1. Draw directly on the screen to learn GDI basics.


