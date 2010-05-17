TOPDIR = ..
include($$TOPDIR/include.pri)

#QT *= multimedia

CONFIG -= release
CONFIG *= debug

TARGET = test_model

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp

SOURCES *= $$TOPDIR/characters.cpp
HEADERS *= $$TOPDIR/characters.h


SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
