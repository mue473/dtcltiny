# dtcltiny.pro - adapted for raidtcl project 2018 - 2025 by Rainer Müller

CONFIG += qt lrelease
TEMPLATE = app
TARGET = dtcltiny
QT += widgets network
PRE_TARGETDEPS += pretarget
QMAKE_EXTRA_TARGETS += timestamp

# Input
HEADERS = config.h \
        aboutdialog.h \
        commandport.h \
        crcfmessage.h \
        hiddencontrollersdialog.h \
        infoport.h \
        listpropertiesdialog.h \
        lococontrol.h \
        locodialog.h \
        mainwindow.h \
        messagehistory.h \
        preferencesdlg.h \
        programmer.h \
        routesviewoptionsdialog.h \
        routeswindow.h \
        serverinfodialog.h \
        srcpmessage.h \
        srcpmessagefactory.h \
        srcpport.h \
        ../icons/dtcltiny_32.xpm

SOURCES = main.cpp \
        aboutdialog.cpp \
        commandport.cpp \
        crcfmessage.cpp \
        hiddencontrollersdialog.cpp \
        infoport.cpp \
        listpropertiesdialog.cpp \
        lococontrol.cpp \
        locodialog.cpp \
        mainwindow.cpp \
        messagehistory.cpp \
        preferencesdlg.cpp \
        programmer.cpp \
        routesviewoptionsdialog.cpp \
        routeswindow.cpp \
        serverinfodialog.cpp \
        srcpmessage.cpp \
        srcpmessagefactory.cpp \
        srcpport.cpp \
        funcsymb.cpp

TRANSLATIONS = \
        translations/dtcltiny_de.ts

timestamp.target = pretarget
timestamp.commands = echo // Timestamp > timestamp.h

# target platform dependent for Windows
win32 {
    contains(QMAKE_HOST.arch, x86_64):{
        message("Host is 64bit")
        OBJECTS_DIR = ../x64/release
        MOC_DIR = $$OBJECTS_DIR
        DESTDIR = $$OBJECTS_DIR
        LRELEASE_DIR = ../x64
    }
    RC_ICONS = ../icons/dtcltiny.ico
    QMAKE_TARGET_COPYRIGHT = 2018 - 2025 © R.Müller
}
# target platform dependent for Linux
else {
    LRELEASE_DIR = .
    QMAKE_LFLAGS += -no-pie

    resources.path = /usr/local/share/dtcltiny
    resources.files = ../data/*
    resources.extra = find $$resources.files -type f -exec chmod 644 '{}' \;

    desktop.path = /usr/share/applications
    desktop.files = ../data/dtcltiny.desktop

    icon.path = /usr/local/share/dtcltiny
    icon.files = ../icons/dtcltiny.ico

    translations.path = /usr/local/share/dtcltiny/translations
    translations.commands = cp *.qm $$translations.path

    documentation.path = /usr/local/share/doc/dtcltiny
    documentation.files = ../doc/*
    documentation.extra = find $$documentation.files -type f -exec chmod 644 '{}' \; 

    manpage.path = /usr/local/share/man
    manpage.files = ../man/*

    target.path = /usr/local/bin
    INSTALLS += documentation manpage translations resources desktop icon target
}
