TOPDIR = ..
MVG_OPTIONS *= --no-model --no-view --no-dialog --no-save
include($$TOPDIR/include.pri)

CONFIG *= release
CONFIG *= debug
QT *= multimedia

TARGET = test_teach

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp
# from http://code.google.com/p/google-diff-match-patch/
SOURCES *= $$TOPDIR/diff_match_patch.cpp

SOURCES *= $$TOPDIR/morse.cpp
HEADERS *= $$TOPDIR/morse.h
SOURCES *= $$TOPDIR/audiooutput.cpp
HEADERS *= $$TOPDIR/audiooutput.h
SOURCES *= $$TOPDIR/teach_morse.cpp
HEADERS *= $$TOPDIR/teach_morse.h

SOURCES *= $$TOPDIR/parse_csv.cpp
MVG_YAML = $$TOPDIR/characters.yaml

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
