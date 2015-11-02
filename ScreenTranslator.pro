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
    LIBS += -L$$PWD/../build/mingw/deps/lib -lws2_32
}
linux{
    QT += x11extras
    INCLUDEPATH += $$PWD/../build/linux/deps/include
    LIBS += -L$$PWD/../build/linux/deps/lib -lX11 -Wl,-rpath,.
}

LIBS += -ltesseract -llept

include(3rd-party/qtsingleapplication/qtsingleapplication.pri)

SOURCES += main.cpp\
    Manager.cpp \
    SettingsEditor.cpp \
    SelectionDialog.cpp \
    GlobalActionHelper.cpp \
    Recognizer.cpp \
    ResultDialog.cpp \
    ProcessingItem.cpp \
    ImageProcessing.cpp \
    LanguageHelper.cpp \
    WebTranslator.cpp \
    WebTranslatorProxy.cpp \
    TranslatorHelper.cpp \
    RecognizerHelper.cpp \
    Utils.cpp \
    Updater.cpp

HEADERS  += \
    Manager.h \
    SettingsEditor.h \
    SelectionDialog.h \
    GlobalActionHelper.h \
    Recognizer.h \
    Settings.h \
    ProcessingItem.h \
    ResultDialog.h \
    ImageProcessing.h \
    LanguageHelper.h \
    WebTranslator.h \
    WebTranslatorProxy.h \
    StAssert.h \
    TranslatorHelper.h \
    RecognizerHelper.h \
    Utils.h \
    Updater.h

FORMS    += \
    SettingsEditor.ui \
    SelectionDialog.ui \
    ResultDialog.ui

RESOURCES += \
    Recources.qrc

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
    uncrustify.cfg\
    .travis.yml \
    TODO.md

QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = Screen Translator
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
VERSION = 2.0.0.0
RC_ICONS = images/icon.ico
