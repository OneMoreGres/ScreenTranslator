# Screen Translator

## Introduction

This software allows you to translate any text on screen.
Basically it is a combination of screen capture, OCR and translation tools.

## Installation

There are 2 versions of the app: general and `compatible`.
They all share the same functionality.
The `compatible` version does not use some hardware optimizations,
but allows the app to be run on elder hardware.

At first, download general version (without any suffixes in
the downloadable archive's name).
If the app will silently crash during its work,
then try the `compatible` version.

**Windows**: download archive from [github releases](https://github.com/OneMoreGres/ScreenTranslator/releases) page, extract it and run `.exe` file.

If the app fails to start with missing dll's error then install `vs_redist*.exe` from the release archive.
If you have any update errors related to SSL/TLS you should also install or repair `vcredist 2010` (from the Microsoft website).

**Linux**: download `.AppImage` file from [github releases](https://github.com/OneMoreGres/ScreenTranslator/releases), make executable (`chmod +x <file>`) and run it.

**OS X**: currently not supported.

## Setup

Start the app, open the updates page of the settings window
and install required recognition languages, translators and, optionally,
hunspell dictionaries.

After languages/translators installation set default recognition and
translation languages, enable some (or all) translators
and the `translate text` setting if needed.

## Usage

1. Run program (note that it doesn't have main window).
2. Press capture hotkey.
3. Select region on screen. Customize it if needed.
4. Get translation of recognized text.
5. Check for updates if something is not working.

## Limitations

* Can not capture some dynamic web-pages/full screen applications

## Dependencies

* see [Qt 5](https://qt-project.org/)
* see [Tesseract](https://github.com/tesseract-ocr/tesseract/)
* see [Leptonica](https://leptonica.com/)
* several online translation services

## Build from source

Look at the scripts (python3) in the `share/ci` folder.
Normally, you should only edit the `config.py` file.

Build dependencies at first, then build the app.

## Attributions

* icons made by
[Smashicons](https://www.flaticon.com/authors/smashicons),
[Freepik](https://www.flaticon.com/authors/freepik),
from [Flaticon](https://www.flaticon.com/)
