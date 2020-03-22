QT = core gui widgets network webenginewidgets

TARGET = screen-translator
TEMPLATE = app
CONFIG += c++17

DEPS_DIR=$$(ST_DEPS_DIR)
isEmpty(DEPS_DIR):DEPS_DIR=$$PWD/../deps
INCLUDEPATH += $$DEPS_DIR/include
LIBS += -L$$DEPS_DIR/lib
LIBS += -ltesseract -lleptonica

win32{
  LIBS += -lUser32
}
linux{
  QT += x11extras
  LIBS += -lX11
}

VER=3.0.0
DEFINES += VERSION="$$VER"
VERSION = $$VER.0
QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = Screen Translator
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
RC_ICONS = $$PWD/share/images/icon.ico

INCLUDEPATH += src src/service src/capture src/ocr \
  src/represent src/translate src/correct

HEADERS += \
  src/capture/capturearea.h \
  src/capture/captureareaselector.h \
  src/capture/capturer.h \
  src/correct/corrector.h \
  src/languagecodes.h \
  src/manager.h \
  src/ocr/recognizer.h \
  src/ocr/recognizerworker.h \
  src/ocr/tesseract.h \
  src/represent/representer.h \
  src/represent/resultwidget.h \
  src/service/apptranslator.h \
  src/service/debug.h \
  src/service/globalaction.h \
  src/service/runatsystemstart.h \
  src/service/singleapplication.h \
  src/service/updates.h \
  src/service/widgetstate.h \
  src/settings.h \
  src/settingseditor.h \
  src/stfwd.h \
  src/substitutionstable.h \
  src/task.h \
  src/translate/translator.h \
  src/translate/webpage.h \
  src/translate/webpageproxy.h \
  src/trayicon.h

SOURCES += \
  src/capture/capturearea.cpp \
  src/capture/captureareaselector.cpp \
  src/capture/capturer.cpp \
  src/correct/corrector.cpp \
  src/languagecodes.cpp \
  src/main.cpp \
  src/manager.cpp \
  src/ocr/recognizer.cpp \
  src/ocr/recognizerworker.cpp \
  src/ocr/tesseract.cpp \
  src/represent/representer.cpp \
  src/represent/resultwidget.cpp \
  src/service/apptranslator.cpp \
  src/service/debug.cpp \
  src/service/globalaction.cpp \
  src/service/runatsystemstart.cpp \
  src/service/singleapplication.cpp \
  src/service/updates.cpp \
  src/service/widgetstate.cpp \
  src/settings.cpp \
  src/settingseditor.cpp \
  src/substitutionstable.cpp \
  src/translate/translator.cpp \
  src/translate/webpage.cpp \
  src/translate/webpageproxy.cpp \
  src/trayicon.cpp

RESOURCES += \
  recources.qrc

FORMS += \
  src/settingseditor.ui

OTHER_FILES += \
  translators/*.js \
  version.json

TRANSLATIONS += \
    share/translations/screentranslator_ru.ts

translations.files = $$PWD/share/translations/screentranslator_ru.qm

translators.files = $$PWD/translators/*.js

linux {
    PREFIX = /usr

    target.path = $$PREFIX/bin

    shortcuts.files = $$PWD/share/screentranslator.desktop
    shortcuts.path = $$PREFIX/share/applications/
    pixmaps.files += $$PWD/share/images/screentranslator.png
    pixmaps.path = $$PREFIX/share/icons/hicolor/128x128/apps/
    translations.path = $$PREFIX/translations

    INSTALLS += target shortcuts pixmaps translations
}
win32 {
    RC_ICONS = $$PWD/share/images/icon.ico
    translations.path = /translations
    target.path = /
    translators.path = /translators
    INSTALLS += target translations translators
}
mac {
    translations.path = Contents/Translations
    QMAKE_BUNDLE_DATA += translations
    ICON = $$PWD/share/images/icon.icns
}
