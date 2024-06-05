/***************************************************************************
                               listpropertiesdialog.h
                             -----------------
    begin                : 2004-10-26
    copyright            : (C) 2004-2008 by Guido Scholz
    email                : guido.scholz@bayernline.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LISTPROPERTIESDIALOG_H
#define LISTPROPERTIESDIALOG_H

#include <qcheckbox.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>


class ListPropertiesDialog: public QDialog {
  Q_OBJECT

  public:
    ListPropertiesDialog(QWidget* parent = 0);
    void setServerData(const QString&, unsigned int);
    void setSrcpBus(unsigned int);
    void setActionData(bool, bool);
    QString getHostname();
    unsigned int getPort();
    unsigned int getSrcpBus();
    bool getServerLogin();
    bool getAutoConnect();
    bool getAutoPowerOn();

  private:
    QLineEdit *hostLE, *portLE, *busLE;
    QCheckBox* autoconnectCB;
    QCheckBox* autopoweronCB;
};

#endif   //LISTPROPERTIESDIALOG_H
