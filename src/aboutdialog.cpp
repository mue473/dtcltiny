// aboutdialog.cpp - adapted for raidtcl project 2018 by Rainer Müller 

/*
 * aboutdialog.cpp
 * ---------------
 * Begin
 *   2007-08-26
 *
 * Copyright
 *   (C) 2007 Guido Scholz <guido-scholz@gmx.net>
 *
 * Description
 *   Dialog window to display program information
 *
 * License
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include <QFont>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "aboutdialog.h"
#include "config.h"
#include "timestamp.h"

/* application icon */
#include "../icons/dtcltiny_48.xpm"


AboutDialog::AboutDialog(QWidget* parent): QDialog(parent)
{
    setModal(true);
    setObjectName("AboutDialog");
    setWindowTitle(tr("About %1").arg(PACKAGE));

    QVBoxLayout* baseLayout = new QVBoxLayout();
    baseLayout->setSpacing(10);

    // first line: pixmap, programname, version
    QHBoxLayout* pixmapLayout = new QHBoxLayout();
    pixmapLayout->setSpacing(20);
    QLabel* pixlabel = new QLabel();
    pixlabel->setPixmap(QPixmap(dtcltiny_48_xpm));
    pixmapLayout->addWidget(pixlabel);

    QLabel* packagelabel = new QLabel(PACKAGE " " VERSION);
    pixmapLayout->addWidget(packagelabel);
    QFont font;
    font.setPointSize(18);
    font.setWeight(QFont::Bold);
    packagelabel->setFont(font);

    pixmapLayout->addStretch();

    // second line: description
    QLabel* desclabel = new QLabel(
            tr("SRCP client to control digital model trains. It is based\n"
                "on \"DigitalTrainControl for Linux\" by Stefan Preis."));

    // third line: web link
    QLabel* linklabel = new QLabel("http://sourceforge.net/projects/dtcltiny");


    // authors groupbox
    QGroupBox* authorGB = new QGroupBox(tr("Authors"));
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setObjectName("vbox");
    authorGB->setLayout(vbox);

    QLabel* mplabel =
        new QLabel("Markus Pfeiffer <dtcltiny@markus-pfeiffer.de>",
            authorGB);
    vbox->addWidget(mplabel);

    QLabel* gslabel =
        new QLabel("Guido Scholz <guido-scholz@gmx.net>",
            authorGB);
    vbox->addWidget(gslabel);

    QLabel* rmlabel =
        new QLabel(tr("modified by Rainer Müller, Version dated ") + __DATE__,
            authorGB);
    vbox->addWidget(rmlabel);

    // OK button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    // arrange layouts
    baseLayout->addLayout(pixmapLayout);
    baseLayout->addWidget(desclabel);
    baseLayout->addWidget(linklabel);
    baseLayout->addWidget(authorGB);
    baseLayout->addWidget(buttonBox);

    setLayout(baseLayout);
}


