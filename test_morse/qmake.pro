TOPDIR = ..
include($$TOPDIR/include.pri)

QT *= multimedia

CONFIG -= release
CONFIG *= debug

TARGET = test_morse

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp

SOURCES *= $$TOPDIR/morse.cpp
HEADERS *= $$TOPDIR/morse.h

SOURCES *= $$TOPDIR/scroller.cpp
HEADERS *= $$TOPDIR/scroller.h

SOURCES *= $$TOPDIR/audiooutput.cpp
HEADERS *= $$TOPDIR/audiooutput.h

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
