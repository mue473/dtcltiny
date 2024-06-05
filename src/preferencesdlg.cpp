// preferencesdlg.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller 

/***************************************************************************
                               preferencesdlg.cpp
                             -------------------
    begin                : 2004-10-26
    copyright            : (C) 2004-2008 by Guido Scholz
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

#include <QFileDialog>
#include <QDialogButtonBox>

#include "preferencesdlg.h"


PreferencesDialog::PreferencesDialog(QWidget* parent): QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Preferences"));
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setObjectName("mainLayout");

    // file groupbox
    QGroupBox *ExtPreferencesGB = 
        new QGroupBox(tr("Preferences"), this);

    QVBoxLayout *extGBLayout = new QVBoxLayout();
    extGBLayout->setObjectName("extGBLayout");
    ExtPreferencesGB->setLayout(extGBLayout);

    // line for autoload option
    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->setObjectName("fileLayout");
    extGBLayout->addLayout(fileLayout);

    autoloadCB = new QCheckBox(tr("&Autoload file on startup"),
            ExtPreferencesGB);
    autoloadCB->setObjectName("autoloadCB");
    fileLayout->addWidget(autoloadCB);
    connect(autoloadCB, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoloadChanged(bool)));

    fileLayout->addStretch();

    filePB = new QPushButton(tr("&Choose..."), ExtPreferencesGB);
    fileLayout->addWidget(filePB);
    connect(filePB, SIGNAL(clicked()), this, SLOT(chooseFile()));

    // line for autoload filename
    fileLE = new QLineEdit(ExtPreferencesGB);
    fileLE->setObjectName("fileLE");
    fileLE->setMinimumWidth(150);
    extGBLayout->addWidget(fileLE);

    // line for mfx master option
    QHBoxLayout *mfxLayout = new QHBoxLayout();
    mfxLayout->setObjectName("mfxLayout");
    extGBLayout->addLayout(mfxLayout);

    mfxMasterCB = new QCheckBox(tr("mf&x master functionality, reregistration counter"),
            ExtPreferencesGB);
    mfxMasterCB->setObjectName("mfxMasterCB");
    mfxLayout->addWidget(mfxMasterCB);
    connect(mfxMasterCB, SIGNAL(toggled(bool)),
            this, SLOT(slotMfxMasterChanged(bool)));

    mfxLayout->addStretch();
    
    rregLE = new QLineEdit(ExtPreferencesGB);
    rregLE->setObjectName("rregLE");
    rregLE->setMinimumWidth(30);
    mfxLayout->addWidget(rregLE);

    // line for scale option
    QHBoxLayout *scaleLayout = new QHBoxLayout();
    scaleLayout->setObjectName("scaleLayout");
    extGBLayout->addLayout(scaleLayout);

    QLabel *scaleLabel = new QLabel(tr("&Rail transport modelling scale"));
    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addStretch();

    scaleCB = new QComboBox(this);
    scaleCB->setObjectName("scaleCB");
    scaleCB->addItem("2 (1:22,5)");  /*0 */
    scaleCB->addItem("1 (1:32)");    /*1 */
    scaleCB->addItem("0 (1:45)");    /*2 */
    scaleCB->addItem("S (1:64)");    /*3 */
    scaleCB->addItem("H0 (1:87)");   /*4 */
    scaleCB->addItem("TT (1:120)");  /*5 */
    scaleCB->addItem("N (1:160)");   /*6 */
    scaleCB->addItem("Z (1:220)");   /*7 */
    scaleLayout->addWidget(scaleCB);
    scaleLabel->setBuddy(scaleCB);

    // OK and Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,
            SLOT(reject()));

    //arrange layout
    mainLayout->addWidget(ExtPreferencesGB);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void PreferencesDialog::chooseFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose file"),
            QDir::homePath(), tr("Loco sets") +
            " (*" + DTCLFILEEXT + ")", NULL, QFileDialog::DontUseNativeDialog);

    if (!filename.isNull())
        fileLE->setText(filename);
}

void PreferencesDialog::setDialogData(bool autoload, const QString& filename,
					bool master, int regcount, int scale)
{
    autoloadCB->setChecked(autoload);
    fileLE->setText(filename);
    mfxMasterCB->setChecked(master);
	rregLE->setText(QString::number(regcount));
    scaleCB->setCurrentIndex(scale);
    slotAutoloadChanged(autoload);
    slotMfxMasterChanged(master);
}

bool PreferencesDialog::getAutoload()
{
    return autoloadCB->isChecked();
}

QString PreferencesDialog::getAutoloadFile()
{
    return fileLE->text();
}

bool PreferencesDialog::getMfxMaster()
{
    return mfxMasterCB->isChecked();
}

unsigned int PreferencesDialog::getRregCntr()
{
    return rregLE->text().toUInt();
}

int PreferencesDialog::getScale()
{
    return scaleCB->currentIndex();
}

void PreferencesDialog::slotAutoloadChanged(bool autoenabled)
{
    filePB->setEnabled(autoenabled);
    fileLE->setEnabled(autoenabled);
}

void PreferencesDialog::slotMfxMasterChanged(bool master)
{
    rregLE->setEnabled(master);
}
