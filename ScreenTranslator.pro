#-------------------------------------------------
#
# Project created by QtCreator 2013-11-22T12:00:23
#
#-------------------------------------------------

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenTranslator
TEMPLATE = app

INCLUDEPATH += ../include

LIBS += -L../bin -ltesseract -llept -ltiff -lgif -ljpeg -lz
LIBS += -lWs2_32

include(3rd-party/qtsingleapplication/qtsingleapplication.pri)

SOURCES += main.cpp\
    Manager.cpp \
    SettingsEditor.cpp \
    SelectionDialog.cpp \
    GlobalActionHelper.cpp \
    Recognizer.cpp \
    Translator.cpp \
    ResultDialog.cpp \
    ProcessingItem.cpp \
    ImageProcessing.cpp \
    LanguageHelper.cpp \
    GoogleWebTranslator.cpp

HEADERS  += \
    Manager.h \
    SettingsEditor.h \
    SelectionDialog.h \
    GlobalActionHelper.h \
    Recognizer.h \
    Translator.h \
    Settings.h \
    ProcessingItem.h \
    ResultDialog.h \
    ImageProcessing.h \
    LanguageHelper.h \
    GoogleWebTranslator.h \
    StAssert.h

FORMS    += \
    SettingsEditor.ui \
    SelectionDialog.ui \
    ResultDialog.ui

RESOURCES += \
    Recources.qrc

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_ru.ts

win32{
RC_FILE = app.rc
}

OTHER_FILES += \
    app.rc \
    images/icon.ico \
    README.md \
    uncrustify.cfg
