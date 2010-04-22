TOPDIR = .
include($$TOPDIR/include.pri)

QT *= multimedia

CONFIG -= release
CONFIG *= debug

SOURCES *= main.cpp

SOURCES *= $$TOPDIR/mydebug.cpp

SOURCES *= morse.cpp
HEADERS *= morse.h

SOURCES *= scroller.cpp
HEADERS *= scroller.h

#SOURCES *= qled.cpp
#HEADERS *= qled.h
#RESOURCES *= qled.qrc

SOURCES *= audiooutput.cpp
HEADERS *= audiooutput.h

SOURCES *= mainwindow.cpp
HEADERS *= mainwindow.h
FORMS   *= mainwindow.ui
