QT       += core gui widgets

CONFIG   += c++17
CONFIG   -= app_bundle

TARGET    = TextEditor
TEMPLATE  = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    finddialog.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    finddialog.h

FORMS += \
    mainwindow.ui
