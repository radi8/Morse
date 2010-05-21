TOPDIR = ..
MVG_OPTIONS *= --no-model --no-view --no-dialog --no-save
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

SOURCES *= $$TOPDIR/parse_csv.cpp
MVG_YAML = $$TOPDIR/characters.yaml

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
