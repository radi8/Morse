UI_DIR      = .obj
MOC_DIR     = .obj
RCC_DIR     = .obj
OBJECTS_DIR = .obj

INCLUDEPATH *= $$TOPDIR

CONFIG -= release
CONFIG *= debug
CONFIG *= warn_on
CONFIG *= silent

TARGET  =  main

QMAKE_CLEAN *= $$DESTDIR/$$TARGET

