/*
 * routesviewoptionsdialog.cpp
 * ------------------------
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

#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QIntValidator>

#include "routesviewoptionsdialog.h"


RoutesViewOptionsDialog::RoutesViewOptionsDialog(QWidget* parent):
    QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Edit routes view options"));

    QVBoxLayout* baseLayout = new QVBoxLayout();
    baseLayout->setSpacing(10);
    setLayout(baseLayout);

    QGroupBox* columnsGB = new QGroupBox(tr("Show columns"));
    QVBoxLayout *columnsLayout = new QVBoxLayout;
    columnsLayout->setObjectName("columnsLayout");
    columnsGB->setLayout(columnsLayout);
    baseLayout->addWidget(columnsGB);

    shownameCB = new QCheckBox(tr("&Name"));
    columnsLayout->addWidget(shownameCB);

    showentryaspectCB = new QCheckBox(tr("&Entry signal aspect"));
    columnsLayout->addWidget(showentryaspectCB);

    showexitaspectCB = new QCheckBox(tr("E&xit signal aspect"));
    columnsLayout->addWidget(showexitaspectCB);

    showlengthCB = new QCheckBox(tr("&Length"));
    columnsLayout->addWidget(showlengthCB);

    showspeedCB = new QCheckBox(tr("&Speed limit"));
    columnsLayout->addWidget(showspeedCB);

    // file loading action groupbox
    QGroupBox* actionGB = new QGroupBox(tr("Number of shown routes"));
    QVBoxLayout* limitLayout = new QVBoxLayout;
    limitLayout->setObjectName("limitLayout");
    actionGB->setLayout(limitLayout);
    baseLayout->addWidget(actionGB);

    limitroutesCB = new QCheckBox(tr("Li&mit number of displayed routes"));
    limitLayout->addWidget(limitroutesCB);

    // line for number label
    QIntValidator* routeValidator = new QIntValidator(2, 999, this);
    QHBoxLayout* numberLayout = new QHBoxLayout();
    numberLayout->setObjectName("numberLayout");
    numberLabel = new QLabel(tr("Num&ber of shown routes"));
    numberLayout->addWidget(numberLabel);
    numberLayout->addStretch();
    numberLE = new QLineEdit();
    numberLE->setMaximumWidth(50);
    numberLE->setValidator(routeValidator);
    numberLayout->addWidget(numberLE);
    numberLabel->setBuddy(numberLE);
    limitLayout->addLayout(numberLayout);

    connect(limitroutesCB, SIGNAL(toggled(bool)),
            this, SLOT(limitRoutesClicked(bool)));

    // OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,
            SLOT(reject()));

    baseLayout->addWidget(buttonBox);
}


void RoutesViewOptionsDialog::setRoutesViewOptions(const
        RoutesViewOptions& options)
{
    shownameCB->setChecked(options.showname);
    showentryaspectCB->setChecked(options.showentryaspect);
    showexitaspectCB->setChecked(options.showexitaspect);
    showlengthCB->setChecked(options.showlength);
    showspeedCB->setChecked(options.showspeed);
    limitroutesCB->setChecked(options.limitroutes);
    numberLE->setText(QString::number(options.routelimit));
    limitRoutesClicked(options.limitroutes);
}


void RoutesViewOptionsDialog::getRoutesViewOptions(
        RoutesViewOptions& options)
{
    options.showname = shownameCB->isChecked();
    options.showentryaspect = showentryaspectCB->isChecked();
    options.showexitaspect = showexitaspectCB->isChecked();
    options.showlength = showlengthCB->isChecked();
    options.showspeed = showspeedCB->isChecked();
    options.limitroutes = limitroutesCB->isChecked();
    options.routelimit = numberLE->text().toUInt();
}


void RoutesViewOptionsDialog::limitRoutesClicked(bool limited)
{
    numberLabel->setEnabled(limited);
    numberLE->setEnabled(limited);
}
