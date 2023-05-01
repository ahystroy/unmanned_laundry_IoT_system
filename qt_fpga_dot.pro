#-------------------------------------------------
#
# Project created by QtCreator 2018-04-04T21:06:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Laundry
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    fpga_dot_font.h \
    server.h

FORMS    += mainwindow.ui


