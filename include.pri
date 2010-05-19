isEmpty(TOPDIR):TOPDIR = .

INCLUDEPATH *= $$TOPDIR

UI_DIR      = .obj
MOC_DIR     = .obj
RCC_DIR     = .obj
OBJECTS_DIR = .obj
MVG_DIR     = .obj

CONFIG -= release
CONFIG *= debug
CONFIG *= warn_on
CONFIG *= silent

TARGET  =  main

QMAKE_CLEAN *= $$DESTDIR/$$TARGET

include($$TOPDIR/mvg.pri)
