[![Build Status](https://travis-ci.org/OneMoreGres/ScreenTranslator.svg)](https://travis-ci.org/OneMoreGres/ScreenTranslator.svg)
[![appveyor](https://img.shields.io/appveyor/ci/OneMoreGres/ScreenTranslator.svg)](https://img.shields.io/appveyor/ci/OneMoreGres/ScreenTranslator.svg)


Screen Translator
=================

Introduction
------------
This software allows you to translate any text on screen.
Basically it is a combination of screen capture, OCR and translation tools.

Usage
-----
1. Press capture hotkey.
2. Select region on screen.
3. Get translation of recognized text.

Features
--------
* Many OCR languages (can be modified dynamically)
* Global hotkeys for main actions
* Copy last translation to clipboard
* Repeat last translation
* Show result in 2 ways (widget or tray baloon)
* Preprocess (scale) recognizeable image
* Interface languages (ru, eng)


Limitations
-----------
* Works only on primary screen
* Can not capture some dynamic web-pages
* Not very precise OCR
* Not all functions are cross-platform (may be bugged on some systems)

Used software
-------------
* see [Qt 5](http://qt-project.org/)
* see [Tesseract](https://code.google.com/p/tesseract-ocr/)
* see [Leptonica](http://leptonica.com/) (Tesseract dependency)
* several online translation services

