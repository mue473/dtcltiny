// main.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
                               main.cpp
                             ------------
    begin                : 11.11.2000
    copyright            : (C) 2000 by Markus Pfeiffer
                         : (C) 2007-2008 by Guido Scholz
    email                : mail@markus-pfeiffer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"
#include "config.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // translation file for Qt library
    QTranslator qt(0);
    QLocale loc = QLocale::system();

    if (qt.load(QString("qt_") + loc.name(),
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#else
                QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#endif
        a.installTranslator(&qt);

    // translation file for application strings
    QTranslator dtcltinyTr(0);
    if (dtcltinyTr.load(QString("dtcltiny_") + loc.name(),
                        TRANSLATIONSDIR))
        a.installTranslator(&dtcltinyTr);

    MainWindow *dtcltinyWindow = new MainWindow();
    Q_CHECK_PTR(dtcltinyWindow);
    dtcltinyWindow->resize(780, 565);

    dtcltinyWindow->show();

    /*only the first application window autoloads a file*/
    //int ac = qApp->argc();

    if (argc == 1)
        dtcltinyWindow->readAutoloadFile();
    else {
        dtcltinyWindow->openFile(argv[1]);
        // loop over all arguments -> open more application windows
        if (argc > 2)
            for (int i = 2; i < argc; i++)
                dtcltinyWindow->openFileWindow(argv[i]);
    }

    return a.exec();
}
