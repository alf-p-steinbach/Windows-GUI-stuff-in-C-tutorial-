@echo off & setlocal
set standard=/EHsc /GR /permissive- /std:c++17 /Zc:__cplusplus /Zc:externC-
set no-silly=/wd4459 /D _CRT_SECURE_NO_WARNINGS=1 /D _STL_SECURE_NO_WARNINGS=1
set CL=/nologo /utf-8 %standard% /W4 %no-silly%

cl %1 user32.lib /link /subsystem:windows /entry:mainCRTStartup
