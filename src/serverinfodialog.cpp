/***************************************************************************
 serverinfodialog.cpp
 --------------------
 Begin      : 2007-08-22
 Copyright  : (C) 2007-2008 by Guido Scholz
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

#include <QGroupBox>
#include <QPushButton>
#include <QLayout>

#include "serverinfodialog.h"


ServerInfoDialog::ServerInfoDialog(QWidget* parent): QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("SRCP server information"));
    QVBoxLayout* baseLayout = new QVBoxLayout();
    setLayout(baseLayout);

    // command session groupbox
    QGroupBox* commandGB = new QGroupBox(tr("Command session"), this);
    baseLayout->addWidget(commandGB);

    // grid layout with 4 rows and 2 columns
    QGridLayout* cmdLayout = new QGridLayout(commandGB);
    
    // 1. line
    cmdLayout->addWidget(new QLabel(tr("Server:"),
                commandGB), 0, 0);
    cmdServerLbl = new QLabel("", commandGB);
    cmdLayout->addWidget(cmdServerLbl, 0, 1);

    // 2. line
    cmdLayout->addWidget(new QLabel(tr("SRCP version:"),
                commandGB), 1, 0);
    cmdSrcpLbl = new QLabel("", commandGB);
    cmdLayout->addWidget(cmdSrcpLbl, 1, 1);

    // 3. line
    QLabel* label = new QLabel(tr("Other SRCP version:"),
                commandGB);
    cmdLayout->addWidget(label, 2, 0);
    //cmdLayout->addWidget(new QLabel(tr("Other SRCP version:"),
    //            commandGB), 2, 0);
    cmdSrcpOtherLbl = new QLabel("", commandGB);
    cmdLayout->addWidget(cmdSrcpOtherLbl, 2, 1);

    // 4. line
    label = new QLabel(tr("SRCP session id:"),
                commandGB);
    cmdLayout->addWidget(label, 3, 0);
    //cmdLayout->addWidget(new QLabel(tr("SRCP session id:"),
    //            commandGB), 3, 0);
    cmdSessionIdLbl = new QLabel("", commandGB);
    cmdLayout->addWidget(cmdSessionIdLbl, 3, 1);


    // info session groupbox
    QGroupBox* infoGB = new QGroupBox(tr("Info session"), this);
    baseLayout->addWidget(infoGB);
    QGridLayout* infoLayout = new QGridLayout(infoGB);

    // 1. line
    infoLayout->addWidget(new QLabel(tr("Server:"),
                infoGB), 0, 0);
    infoServerLbl = new QLabel("", infoGB);
    infoLayout->addWidget(infoServerLbl, 0, 1);

    // 2. line
    infoLayout->addWidget(new QLabel(tr("SRCP version:"),
                infoGB) , 1, 0);
    infoSrcpLbl = new QLabel("", infoGB);
    infoLayout->addWidget(infoSrcpLbl, 1, 1);

    // 3. line
    infoLayout->addWidget(new QLabel(tr("Other SRCP version:"),
                infoGB), 2, 0);
    infoSrcpOtherLbl = new QLabel("", infoGB);
    infoLayout->addWidget(infoSrcpOtherLbl, 2, 1);

    // 4. line
    infoLayout->addWidget(new QLabel(tr("SRCP session id:"),
                infoGB) , 3, 0);
    infoSessionIdLbl = new QLabel("", infoGB);
    infoLayout->addWidget(infoSessionIdLbl, 3, 1);


    // OK button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    baseLayout->addLayout(buttonLayout);
    buttonLayout->addStretch();

    QPushButton* okPB = new QPushButton(tr("OK"), this);
    buttonLayout->addWidget(okPB);
    okPB->setDefault(true);
    connect(okPB, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout->addStretch();
}

/*
 * set data of command session
 */
void ServerInfoDialog::setCommandSessionData(const QString& server,
        const QString& srcp, const QString& srcpother,
        unsigned int sessionid)
{
    cmdServerLbl->setText(server);
    cmdSrcpLbl->setText(srcp);
    cmdSrcpOtherLbl->setText(srcpother);
    cmdSessionIdLbl->setText(QString::number(sessionid));
}

/*
 * set data of info session
 */
void ServerInfoDialog::setInfoSessionData(const QString& server,
        const QString& srcp, const QString& srcpother,
        unsigned int sessionid)
{
    infoServerLbl->setText(server);
    infoSrcpLbl->setText(srcp);
    infoSrcpOtherLbl->setText(srcpother);
    infoSessionIdLbl->setText(QString::number(sessionid));
}

