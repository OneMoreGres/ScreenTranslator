@echo off

set SELF_PATH=%~dp0
call %SELF_PATH%\env.bat

rem choco install curl cmake

mkdir download

if "%WITH_TESSDATA%" == ""  goto libs
echo "Downloading tessdata"
mkdir download\tessdata
cd download\tessdata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.bigrams
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.fold
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.lm
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.params
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.size
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.cube.word-freq
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/ara.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/chi_sim.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/deu.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.bigrams
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.fold
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.lm
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.params
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.size
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.cube.word-freq
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.tesseract_cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/eng.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.bigrams
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.fold
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.lm
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.params
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.size
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.cube.word-freq
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.tesseract_cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/fra.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/frk.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/frm.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/jpn.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.fold
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.lm
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.params
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.size
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.cube.word-freq
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/rus.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.bigrams
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.fold
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.lm
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.nn
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.params
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.size
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.cube.word-freq
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa.traineddata
curl -fsSLk -O   https://github.com/tesseract-Ocr/tessdata/raw/3.04.00/spa_old.traineddata
cd ..\..
:libs


if "%CLEAR_CACHE%" == ""  goto build-libs
echo "Clearing cache"
rmdir /s /q leptonica
rmdir /s /q leptonica-build
rmdir /s /q tesseract
rmdir /s /q leptonica-build
rmdir /s /q installed
:build-libs


if exist installed\bin\tesseract*.dll goto end

echo "Downloading dependencies"
if not exist download\leptonica.zip (
    curl -fsSLk -o download\leptonica.zip https://github.com/DanBloomberg/leptonica/archive/1.74.4.zip
)
if not exist download\tesseract.zip (
    curl -fsSLk -o download\tesseract.zip https://github.com/tesseract-Ocr/tesseract/archive/3.05.01.zip
) 


echo "Building dependencies"
unzip -qq download\leptonica.zip
move leptonica* leptonica
mkdir leptonica-build
cd leptonica-build
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_CPU=%ARCH% -DCMAKE_INSTALL_PREFIX=..\installed ..\leptonica 
cmake --build . --config Release
cmake --build . --config Release --target install
cd ..
copy /y /b installed\lib\leptonica*.lib installed\lib\lept.lib
if %errorlevel% neq 0 exit /b %errorlevel%


unzip -qq download\tesseract.zip
move tesseract* tesseract
mkdir tesseract-build
cd tesseract-build
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_CPU=%ARCH% -DCMAKE_INSTALL_PREFIX=..\installed -DCMAKE_PREFIX_PATH=..\installed ..\tesseract 
cmake --build . --config Release
cmake --build . --config Release --target install
cd ..
copy /y /b installed\lib\tesseract*.lib installed\lib\tesseract.lib
if %errorlevel% neq 0 exit /b %errorlevel%

:end

