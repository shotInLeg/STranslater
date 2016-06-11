#-------------------------------------------------
#
# Project created by QtCreator 2016-06-10T13:44:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = STranslater
TEMPLATE = app


SOURCES += main.cpp\
        stranslater.cpp \
    Translator/translator.cpp

HEADERS  += stranslater.h \
    Translator/translator.h

FORMS    += stranslater.ui

RESOURCES += \
    lang.qrc
