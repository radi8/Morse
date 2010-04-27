TOPDIR = ..
include($$TOPDIR/include.pri)

CONFIG *= release
CONFIG *= debug

TARGET = test_teach

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp
# from http://code.google.com/p/google-diff-match-patch/
SOURCES *= $$TOPDIR/diff_match_patch.cpp

SOURCES *= $$TOPDIR/morse.cpp
HEADERS *= $$TOPDIR/morse.h
SOURCES *= $$TOPDIR/teach_morse.cpp
HEADERS *= $$TOPDIR/teach_morse.h

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
