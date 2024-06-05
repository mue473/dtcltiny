/***************************************************************************
 messagehistory.h
 ------------------
 Begin      : 2007-08-20
 Copyright  : (C) 2007 by Guido Scholz
 E-Mail     : guido-scholz@gmx.net
 Description: Stacked set of Combo Boxex to collect different message types
              in separated lists.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGEHISTORY_H
#define MESSAGEHISTORY_H

#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>


class MessageHistory: public QWidget {

Q_OBJECT

public:
    MessageHistory(QWidget* parent = 0);
    void enableTime(bool);
    bool isTimeEnabled();

public slots:
    void toggleLine();
    void addHint(const QString&);
    void addCommand(const QString&);
    void addInfo(const QString&);

private:
    enum VisibleLine{vlHint = 0, vlCommand, vlInfo};
    VisibleLine visibleLine;
    bool showTime;

    QStackedWidget* cbStack;
    QStackedWidget* lblStack;
    QComboBox* commandCB;
    QComboBox* hintCB;
    QComboBox* infoCB;
    QString getCurrentTime();
    void saveLog();

protected:
    void mousePressEvent(QMouseEvent*);
};

#endif                          //MESSAGEHISTORY_H
