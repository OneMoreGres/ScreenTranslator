@echo off

set PATH=c:\Program Files (x86)\WinSCP\;%PATH%

set SELF_PATH=%~dp0
set ROOT=%SELF_PATH%\..\..


for /f "delims=" %%i in ('findstr versionString %ROOT%\version.json') do set VERSION_LINE=%%i
set UNQUOTED=%VERSION_LINE:"='%
for /f "tokens=4 delims='" %%i in ("%UNQUOTED%") do set VERSION=%%i

for /f "delims=" %%i in ('dir /b screen-translator-online*.exe') do set online=%%i
for /f "delims=" %%i in ('dir /b screen-translator-offline*.exe') do set offline=%%i


winscp.com /keygen %SELF_PATH%\sf_key /output=key.ppk
set folder="/home/frs/project/screen-translator/bin/v%VERSION%"
set bin_folder="/home/frs/project/screen-translator/bin"
winscp.com /command "open sftp://onemoregres@frs.sourceforge.net/ -privatekey=key.ppk -hostkey=*" ^
 "mkdir %folder%" ^
 "put %online% %folder%/%online%" ^
 "put %offline% %folder%/%offline%" ^
 "put %ROOT%\distr\Changelog_en.txt %bin_folder%/readme.md" ^
 "put %ROOT%\distr\Changelog_ru.txt %bin_folder%/readme_ru.md" ^
 "exit"


set url="https://sourceforge.net/projects/screen-translator/files/bin/v%VERSION%/%online%"
curl --insecure -H "Accept: application/json" -X PUT -d "default=windows" -d "api_key=%sf_api%" %url%

set url="https://sourceforge.net/projects/screen-translator/files/bin/v%VERSION%/%offline%"
curl --insecure -H "Accept: application/json" -X PUT -d "default=windows" -d "api_key=%sf_api%" %url%
