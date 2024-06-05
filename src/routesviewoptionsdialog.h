/*
 * routesviewoptionsdialog.h
 * -------------------------
 * Begin    : 2016-05-14
 * Copyright: (C) 2016 by Guido Scholz
 * e-mail   : guido-scholz@gmx.net
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ROUTESVIEWOPTIONSDIALOG_H
#define ROUTESVIEWOPTIONSDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>


//container for dialog data
struct RoutesViewOptions{
    bool showname;
    bool showentryaspect;
    bool showexitaspect;
    bool showlength;
    bool showspeed;
    bool limitroutes;
    unsigned int routelimit;
};


class RoutesViewOptionsDialog: public QDialog {
  Q_OBJECT

    QCheckBox* shownameCB;
    QCheckBox* showentryaspectCB;
    QCheckBox* showexitaspectCB;
    QCheckBox* showlengthCB;
    QCheckBox* showspeedCB;
    QCheckBox* limitroutesCB;
    QLabel* numberLabel;
    QLineEdit *numberLE;

  public:
    RoutesViewOptionsDialog(QWidget* parent = 0);
    void setRoutesViewOptions(const RoutesViewOptions&);
    void getRoutesViewOptions(RoutesViewOptions&);

  private slots:
    void limitRoutesClicked(bool);
};

#endif   //ROUTESVIEWOPTIONSDIALOG_H
