@echo off

if "%ARCH%" == "" set ARCH=x86

if /i %ARCH% == x86       goto x86
if /i %ARCH% == x64       goto x64
goto end

:x64
call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64
set PATH=c:\Qt\5.5\msvc2013_64\bin\;c:\Program Files (x86)\NSIS\;C:\Program Files (x86)\Inno Setup 5;C:\Program Files\CMake\bin;%PATH%
goto end

:x86
call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
set PATH=c:\Qt\5.5\msvc2013\bin\;c:\Program Files (x86)\NSIS\;C:\Program Files (x86)\Inno Setup 5;C:\Program Files\CMake\bin;%PATH%
goto end

:end
