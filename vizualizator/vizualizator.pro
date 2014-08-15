#-------------------------------------------------
#
# Project created by QtCreator 2014-08-10T10:57:45
#
#-------------------------------------------------
BASE_CXX = $$QMAKE_CXX
system(ccache -V):QMAKE_CXX = ccache $$BASE_CXX

QT       += core gui
QT       += widgets multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

!android {
  exists( /usr/include/opencv2/opencv.hpp ) {
    message(" -> Utilisation de OpenCV 2 (precise)")
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect
    DEFINES += WITH_OPENCV2
  }
}

TARGET = vizualizator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    vizualizatorwidget.cpp \
    valueddial.cpp \
    lib/vizualizatorimage.cpp

HEADERS  += mainwindow.h \
    vizualizatorwidget.h \
    valueddial.h \
    lib/vizualizatorimage.h

FORMS    += mainwindow.ui \
    vizualizatorwidget.ui

###############################################################################
#               GESTION LOCATION OBJETS COMPILES (shadow/.build)
###############################################################################
unix {
     # quand on est pas en shadow build
     OBJECTS_DIR = $$PWD/../.build/
     MOC_DIR     = $$PWD/../.build/
     RCC_DIR     = $$PWD/../.build/
     UI_DIR      = $$PWD/../.build/
  } else {
     #on est en shadow build
     #message("On est en shadow build je ne specifie donc pas le .build comme repertoire de stockage des fichiers temp")
  }
