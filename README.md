# Screen Translator

## Introduction

This software allows you to translate any text on screen.
Basically it is a combination of screen capture, OCR and translation tools.

## Installation

**Windows**: download and run the installer from [github releases](https://github.com/OneMoreGres/ScreenTranslator/releases) page.

**Linux**: install dependencies and build from source (edit .pro, qmake && make).
Also download [data files](https://github.com/tesseract-ocr/tessdata/tree/3.04.00) for tesseract.

**OS X**: currently not supported.

## Usage

1. Run program (note that it doesn't have main window).
2. Press capture hotkey.
3. Select region on screen.
4. Get translation of recognized text.
5. Check for updates if something not working.

## Features

* Many OCR languages (can be modified dynamically)
* Global hotkeys for main actions
* Copy last translation to clipboard
* Repeat last translation
* Show result in 2 ways (widget or tray baloon)
* Preprocess (scale) recognizeable image
* Interface languages (ru, eng)

## Limitations

* Can not capture some dynamic web-pages
* Not very precise OCR
* Not all functions are cross-platform (may be bugged on some systems)

## Dependencies

* see [Qt 5](http://qt-project.org/)
* see [Tesseract](https://code.google.com/p/tesseract-ocr/)
* see [Leptonica](http://leptonica.com/)
* several online translation services

## Attributions

* icons made by [Smashicons](https://www.flaticon.com/authors/smashicons)
from [Flaticon](https://www.flaticon.com/)
