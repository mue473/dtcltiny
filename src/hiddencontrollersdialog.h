/***************************************************************************
 hiddencontrollersdialog.h
 -----------------
 Begin      : 2016-01-10
 Copyright  : (C) 2016 by Guido Scholz
 E-Mail     : guido-scholz@gmx.net
 Description: Dialog window to display hidden controllers
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HIDDENCONTROLLERSDIALOG_H
#define HIDDENCONTROLLERSDIALOG_H

#include <QDialog>
//#include <QListWidget>
#include <QPushButton>
#include <QTreeWidget>

#include "lococontrol.h"


class HiddenControllersDialog: public QDialog {
    Q_OBJECT

    QPushButton* showButton;
	QTreeWidget	*listWidget;
    

public:
    HiddenControllersDialog(QWidget* parent = 0);

    void setControllersList(const QList<LocoControl *>& hl);

public slots:
    void slotShowSelectedController();

signals:
    void showController(int);

};

#endif    //HIDDENCONTROLLERSDIALOG_H

