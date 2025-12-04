QT       += core gui widgets sql
QT += printsupport

CONFIG += c++17

TARGET = ReceptionSystem
TEMPLATE = app

SOURCES += main.cpp \
           filterresult.cpp \
           login.cpp \
           menu.cpp

HEADERS += login.h \
           filterresult.h \
           menu.h

FORMS += login.ui \
         filterresult.ui \
         menu.ui
