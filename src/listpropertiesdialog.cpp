/*
 * listpropertiesdialog.cpp
 * ------------------------
 * Begin    : 2004-08-27
 * Copyright: (C) 2007-2008 by Guido Scholz
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

#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QIntValidator>

#include "listpropertiesdialog.h"


ListPropertiesDialog::ListPropertiesDialog(QWidget* parent): QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("SRCP server properties"));

    QVBoxLayout* baseLayout = new QVBoxLayout();
    baseLayout->setSpacing(10);
    setLayout(baseLayout);

    QGroupBox* serverGB = new QGroupBox(tr("SRCP-Server"));
    QVBoxLayout *serverLayout = new QVBoxLayout;
    serverLayout->setObjectName("serverLayout");
    serverGB->setLayout(serverLayout);
    baseLayout->addWidget(serverGB);

    // line for hostname
    QHBoxLayout* hostLayout = new QHBoxLayout();
    hostLayout->setObjectName("hostLayout");
    QLabel* hostLabel = new QLabel(tr("&Hostname (IP or DNS)"));
    hostLayout->addWidget(hostLabel);
    hostLayout->addStretch();
    hostLE = new QLineEdit();
    hostLE->setMinimumWidth(160);
    hostLayout->addWidget(hostLE);
    hostLabel->setBuddy(hostLE);
    serverLayout->addLayout(hostLayout);

    QIntValidator* portValidator = new QIntValidator(1, 65535, this);
    // line for port number
    QHBoxLayout *portLayout = new QHBoxLayout();
    portLayout->setObjectName("portLayout");
    QLabel* portLabel = new QLabel(tr("&Port (1-65535)"));
    portLayout->addWidget(portLabel);
    portLayout->addStretch();
    portLE = new QLineEdit();
    portLE->setMinimumWidth(160);
    portLE->setValidator(portValidator);
    portLayout->addWidget(portLE);
    portLabel->setBuddy(portLE);
    serverLayout->addLayout(portLayout);

    QIntValidator* busValidator = new QIntValidator(1, 999, this);
    // line for bus number
    QLabel *busLabel = new QLabel(tr("&SRCP-Bus"));
    busLE = new QLineEdit();
    busLE->setMaxLength(4);
    busLE->setMinimumWidth(160);
    busLE->setValidator(busValidator);
    busLabel->setBuddy(busLE);
    QHBoxLayout *busLayout = new QHBoxLayout();

    busLayout->addWidget(busLabel);
    busLayout->addStretch();
    busLayout->addWidget(busLE);
    serverLayout->addLayout(busLayout);


    // file loading action groupbox
    QGroupBox* actionGB = new QGroupBox(tr("Actions on file loading"));
    QVBoxLayout* actionLayout = new QVBoxLayout;
    actionLayout->setObjectName("actionLayout");
    actionGB->setLayout(actionLayout);
    baseLayout->addWidget(actionGB);

    // line for autologin checkbox
    autoconnectCB = new QCheckBox(tr("Autoconnect to &server"));
    actionLayout->addWidget(autoconnectCB);

    // line for autopoweron checkbox
    autopoweronCB = new QCheckBox(tr("Autostart layout &voltage"));
    actionLayout->addWidget(autopoweronCB);

    // OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,
            SLOT(reject()));

    baseLayout->addWidget(buttonBox);
}


void ListPropertiesDialog::setServerData(const QString& hostname,
        unsigned int portnumber)
{
    hostLE->setText(hostname);
    portLE->setText(QString::number(portnumber));
}


void ListPropertiesDialog::setActionData(bool autoconnect,
        bool autopoweron)
{
    autoconnectCB->setChecked(autoconnect);
    autopoweronCB->setChecked(autopoweron);
}


void ListPropertiesDialog::setSrcpBus(unsigned int bus)
{
    busLE->setText(QString::number(bus));
}


QString ListPropertiesDialog::getHostname()
{
    return hostLE->text();
}


unsigned int ListPropertiesDialog::getPort()
{
    return portLE->text().toUInt();
}


unsigned int ListPropertiesDialog::getSrcpBus()
{
    return busLE->text().toUInt();
}


bool ListPropertiesDialog::getAutoConnect()
{
    return autoconnectCB->isChecked();
}


bool ListPropertiesDialog::getAutoPowerOn()
{
    return autopoweronCB->isChecked();
}

