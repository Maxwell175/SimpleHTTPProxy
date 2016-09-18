QT += core
QT += network
QT -= gui

CONFIG += c++11

TARGET = SimpleHTTPProxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    httpserver.cpp \
    common.cpp \
    configmanager.cpp \
    httpclient.cpp

HEADERS += \
    httpserver.h \
    common.h \
    configmanager.h \
    httpclient.h
