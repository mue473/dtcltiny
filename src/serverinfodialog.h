/***************************************************************************
 serverinfodialog.h
 -----------------
 Begin      : 2007-08-22
 Copyright  : (C) 2007 by Guido Scholz
 E-Mail     : guido.scholz@bayernline.de
 Description: Dialog window to display SRCP server information
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SERVERINFODIALOG_H
#define SERVERINFODIALOG_H

#include <qdialog.h>
#include <qlabel.h>


class ServerInfoDialog: public QDialog {
    Q_OBJECT
   
    QLabel* cmdServerLbl;
    QLabel* cmdSrcpLbl;
    QLabel* cmdSrcpOtherLbl;
    QLabel* cmdSessionIdLbl;

    QLabel* infoServerLbl;
    QLabel* infoSrcpLbl;
    QLabel* infoSrcpOtherLbl;
    QLabel* infoSessionIdLbl;

public:
    ServerInfoDialog(QWidget* parent = 0);
    void setCommandSessionData(const QString&, const QString&,
                       const QString&, unsigned int);
    void setInfoSessionData(const QString&, const QString&,
                       const QString&, unsigned int);
};

#endif    //SERVERINFODIALOG_H
