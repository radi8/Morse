TOPDIR = ..
include($$TOPDIR/include.pri)


CONFIG -= release
CONFIG *= debug

TARGET = test_model

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp

SOURCES *= $$TOPDIR/parse_csv.cpp
MVG_YAML = $$TOPDIR/characters.yaml

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
