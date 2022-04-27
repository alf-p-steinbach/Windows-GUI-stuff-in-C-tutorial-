@echo off & setlocal
md .build 2>nul
pushd .build
rc /nologo /c 65001 /Fo"resources.res" ..\resources.rc

set CL=^
  /nologo /utf-8 /EHsc /GR /permissive- /std:c++17 /Zc:__cplusplus /Zc:externC- /W4 /wd4459 ^
  /D _CRT_SECURE_NO_WARNINGS=1 /D _STL_SECURE_NO_WARNINGS=1
set LINK=/entry:mainCRTStartup
cl ..\main.cpp user32.lib resources.res /link /subsystem:windows /out:"hello.exe"
popd
