#-------------------------------------------------
#
# Project created by QtCreator 2013-11-22T12:00:23
#
#-------------------------------------------------

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenTranslator
TEMPLATE = app
CONFIG += c++11

win32{
    INCLUDEPATH += $$PWD/../build/mingw/deps/include
    LIBS += -L$$PWD/../build/mingw/deps/lib -lUser32 -lws2_32
}
linux{
    QT += x11extras
    INCLUDEPATH += $$PWD/../build/linux/deps/include
    LIBS += -L$$PWD/../build/linux/deps/lib -lX11 -Wl,-rpath,.
}

LIBS += -ltesseract -llept

include(3rd-party/qtsingleapplication/qtsingleapplication.pri)

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/manager.cpp \
    src/settingseditor.cpp \
    src/selectiondialog.cpp \
    src/globalactionhelper.cpp \
    src/recognizer.cpp \
    src/resultdialog.cpp \
    src/processingitem.cpp \
    src/imageprocessing.cpp \
    src/languagehelper.cpp \
    src/webtranslator.cpp \
    src/webtranslatorproxy.cpp \
    src/translatorhelper.cpp \
    src/recognizerhelper.cpp \
    src/utils.cpp \
    src/updater.cpp

HEADERS += \
    src/manager.h \
    src/settingseditor.h \
    src/selectiondialog.h \
    src/globalactionhelper.h \
    src/recognizer.h \
    src/settings.h \
    src/processingitem.h \
    src/resultdialog.h \
    src/imageprocessing.h \
    src/languagehelper.h \
    src/webtranslator.h \
    src/webtranslatorproxy.h \
    src/stassert.h \
    src/translatorhelper.h \
    src/recognizerhelper.h \
    src/utils.h \
    src/updater.h

FORMS += \
    src/settingseditor.ui \
    src/selectiondialog.ui \
    src/resultdialog.ui

RESOURCES += \
    recources.qrc

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_ru.ts

OTHER_FILES += \
    images/* \
    translators/* \
    scripts/* \
    distr/* \
    version.json \
    README.md \
    uncrustify.cfg \
    .travis.yml \
    TODO.md

QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = Screen Translator
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
VERSION = 2.0.0.0
RC_ICONS = images/icon.ico
