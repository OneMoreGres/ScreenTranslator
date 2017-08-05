@echo off

set SELF_PATH=%~dp0
call %SELF_PATH%\env.bat


::build
rmdir /q /s build
set ROOT=%SELF_PATH%\..\..
lrelease %ROOT%\ScreenTranslator.pro
mkdir build
cd build
qmake INCLUDEPATH+=%cd%\..\installed\include LIBS+=-L%cd%\..\installed\bin LIBS+=-L%cd%\..\installed\lib %ROOT%
nmake
cd ..
if %errorlevel% neq 0 exit /b %errorlevel%


::pack
rmdir /q /s iss
mkdir iss
copy /Y %ROOT%\distr\iss\* iss
copy /Y %ROOT%\distr\Changelog_en.txt iss\
copy /Y %ROOT%\distr\Changelog_ru.txt iss\
copy /Y %ROOT%\LICENSE.md iss\LICENSE_en.md
copy /Y %ROOT%\images\icon.ico iss\icon.ico

mkdir iss\content
copy /Y build\release\ScreenTranslator.exe iss\content\ScreenTranslator.exe
copy /Y installed\bin\*.dll iss\content
mkdir iss\content\translations
copy /Y %ROOT%\translations\*.qm iss\content\translations
mkdir iss\content\translators
copy /Y %ROOT%\translators\* iss\content\translators

windeployqt --release iss\content\ScreenTranslator.exe

for /f "delims=" %%i in ('findstr versionString %ROOT%\version.json') do set VERSION_LINE=%%i
set UNQUOTED=%VERSION_LINE:"='%
for /f "tokens=4 delims='" %%i in ("%UNQUOTED%") do set VERSION=%%i
echo #define MyAppVersion "%VERSION%" > iss\defines.iss

 

cd iss
iscc.exe InnoSetup.iss
cd ..


if "%WITH_TESSDATA%" == ""  goto end

mkdir iss\tessdata
copy /Y download\tessdata\* iss\tessdata

cd iss
iscc.exe InnoSetupWithTessdata.iss
cd ..

:end