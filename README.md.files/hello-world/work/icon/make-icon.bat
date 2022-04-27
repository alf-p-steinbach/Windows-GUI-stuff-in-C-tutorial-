@echo off & setlocal
if not exist m:magick.exe (
    echo.!Oops: m:magick.exe ^(from the Image Magick tools^) not found. >&2
    exit /b 1
)
rem set IMAGES=512px.png 256px.png 128px.png 64px.png 32px.png 16px.png
set IMAGES=256px.png 128px.png 64px.png 32px.png 16px.png
pushd images
m:magick %IMAGES% main.ico
move main.ico .. >nul
popd
dir /b main.ico
