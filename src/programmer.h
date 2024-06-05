/***************************************************************************
                               Programmer.h
                             -----------------
    begin                : 11.11.2000
    copyright            : (C) 2000 by Markus Pfeiffer
    copyright            : (C) 2008 by Guido Scholz
    email                : guido-scholz@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QButtonGroup>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>

#include "srcpmessage.h"


enum PrgrmID {NMRA, UHL};

class Programmer: public QDialog {
  Q_OBJECT

    int iType;
    bool bStarted;
    unsigned int srcpbus;

    QButtonGroup *actionGroup;
    QLabel *labelCV_Reg;
    QLabel *labelBit;
    QLabel *labelValue;
    QLabel *labelAddress;
    QLineEdit *leCV_Reg;
    QLineEdit *leBit;
    QLineEdit *leValue;
    QLineEdit *leAddress;
    QProgressBar *progress;

public:
    Programmer(QWidget* parent, int type, bool started, unsigned int bus);

private slots:
    void slotProgramNMRA();
    void slotProgramUHL();
    void slotSetupChoice(int);

signals:
    void sendSrcpMessage(const SrcpMessage&);
};

#endif                          //PROGRAMMER_H
