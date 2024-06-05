/***************************************************************************
                               programmer.cpp
                             -------------------
    begin                : 11.11.2000
    copyright            : (c) 2000 by Markus Pfeiffer
                         : (c) 2008 Guido Scholz
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

#include <QLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>
#include <QRadioButton>
#include <QPushButton>
#include <QThread>

#include "programmer.h"


class SleeperThread : public QThread
{
    public:
        static void sleep(unsigned long msecs)
        {
            QThread::sleep(msecs);
        }
        static void usleep(unsigned long msecs)
        {
            QThread::usleep(msecs);
        }
};


enum progAction {
    progCv = 0, progCvBit, progReg, progNmra
};


Programmer::Programmer(QWidget* parent, int type, bool started,
        unsigned int bus):
    QDialog(parent), iType(type), bStarted(started),
    srcpbus(bus)
{
    setModal(true);

    if (iType == NMRA)
        setWindowTitle(tr("Programmer for NMRA-Decoders"));
    else
        setWindowTitle(tr("Programmer for Uhlenbrock-Decoders"));

    QVBoxLayout* baseLayout = new QVBoxLayout();
    setLayout(baseLayout);

    QHBoxLayout* leftrightLayout = new QHBoxLayout();
    baseLayout->addLayout(leftrightLayout);

    // action group box
    QGroupBox* choiceGB = new QGroupBox(tr("Action"), this);
    QVBoxLayout* choiceLayout = new QVBoxLayout();
    choiceGB->setLayout(choiceLayout);
    leftrightLayout->addWidget(choiceGB);

    actionGroup = new QButtonGroup(this);

    QRadioButton *rbCV_Set = new QRadioButton(tr("Set a &CV"), this);
    choiceLayout->addWidget(rbCV_Set );
    actionGroup->addButton(rbCV_Set, progCv);

    QRadioButton *rbCVbit_Set =
        new QRadioButton(tr("Set one &bit of a CV"), this);
    choiceLayout->addWidget(rbCVbit_Set);
    actionGroup->addButton(rbCVbit_Set, progCvBit);

    QRadioButton *rbReg_Set =
        new QRadioButton(tr("Set a &register"), this);
    choiceLayout->addWidget(rbReg_Set);
    actionGroup->addButton(rbReg_Set, progReg);

    QRadioButton *rbNMRA_Set = new QRadioButton(
            tr("Set decoder to &NMRA only"), this);
    choiceLayout->addWidget(rbNMRA_Set);
    actionGroup->addButton(rbNMRA_Set, progNmra);

    connect(actionGroup, SIGNAL(buttonClicked(int)), this,
            SLOT(slotSetupChoice(int)));


    // parameter group box
    QGroupBox* paramGroup = new QGroupBox(tr("Parameters"), this);
    QVBoxLayout *paramGBL = new QVBoxLayout();
    paramGroup->setLayout(paramGBL);

    leftrightLayout->addWidget(paramGroup);

    // 4 line with Address
    QHBoxLayout* addressLayout = new QHBoxLayout();
    labelAddress = new QLabel(tr("&Address:"), paramGroup);

    leAddress = new QLineEdit(paramGroup);
    leAddress->setMaxLength(3);
    labelAddress->setBuddy(leAddress);

    addressLayout->addWidget(labelAddress);
    addressLayout->addStretch();
    addressLayout->addWidget(leAddress);
    paramGBL->addLayout(addressLayout);

    // 1 line with CV No
    QHBoxLayout* cvLayout = new QHBoxLayout();
    labelCV_Reg = new QLabel("C&V #:", paramGroup);

    leCV_Reg = new QLineEdit(paramGroup);
    leCV_Reg->setMaxLength(4);
    labelCV_Reg->setBuddy(leCV_Reg);

    cvLayout->addWidget(labelCV_Reg);
    cvLayout->addStretch();
    cvLayout->addWidget(leCV_Reg);
    paramGBL->addLayout(cvLayout);

    // 2 line with Bit No
    QHBoxLayout* bitLayout = new QHBoxLayout();
    labelBit = new QLabel(tr("Bi&t #:"), paramGroup);

    leBit = new QLineEdit(paramGroup);
    leBit->setMaxLength(1);
    labelBit->setBuddy(leBit);

    bitLayout->addWidget(labelBit);
    bitLayout->addStretch();
    bitLayout->addWidget(leBit);
    paramGBL->addLayout(bitLayout);

    // 3 line with Value
    QHBoxLayout* valueLayout = new QHBoxLayout();
    labelValue = new QLabel(tr("Val&ue:"), paramGroup);

    leValue = new QLineEdit(paramGroup);
    leValue->setMaxLength(3);
    labelValue->setBuddy(leValue);

    valueLayout->addWidget(labelValue);
    valueLayout->addStretch();
    valueLayout->addWidget(leValue);
    paramGBL->addLayout(valueLayout);

    // line with programming button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton *programm = new QPushButton(tr("&Program"));
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
                            QDialogButtonBox::Close);
    buttonBox->addButton(programm, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // add progress bar for UHL programmer
    progress = new QProgressBar();
    progress->hide();

    buttonLayout->addWidget(progress);
    buttonLayout->addWidget(buttonBox);
    baseLayout->addLayout(buttonLayout);

    if (iType == UHL) {
        labelCV_Reg->setText(tr("&Register #:"));
        rbCV_Set->setEnabled(false);
        rbCVbit_Set->setEnabled(false);
        rbNMRA_Set->setEnabled(false);
        connect(programm, SIGNAL(clicked()), this, SLOT(slotProgramUHL()));
        rbReg_Set->setChecked(true); 
        slotSetupChoice(2);
    }
    else {
        connect(programm, SIGNAL(clicked()), this,
                SLOT(slotProgramNMRA()));
        rbCV_Set->setChecked(true);
        slotSetupChoice(0);
    }
}

// sets up the right window half in respect to the left one
void Programmer::slotSetupChoice(int button)
{
    labelValue->setEnabled(button != 3);
    leValue->setEnabled(button != 3);

    leCV_Reg->clear();
    leValue->clear();
    leBit->clear();
    //leAddress->clear();

    switch (button) {
        // CV set
        case progCv:
            labelCV_Reg->setText(tr("&CV #:"));
            labelBit->setEnabled(false);
            leBit->setEnabled(false);
            labelCV_Reg->setEnabled(true);
            leCV_Reg->setEnabled(true);
            break;

            // CV bit set
        case progCvBit:
            labelCV_Reg->setText(tr("&CV #:"));
            labelBit->setEnabled(true);
            leBit->setEnabled(true);
            labelCV_Reg->setEnabled(true);
            leCV_Reg->setEnabled(true);
            break;

            // Register set
        case progReg:
            labelCV_Reg->setText(tr("&Register #:"));
            labelBit->setEnabled(false);
            leBit->setEnabled(false);
            labelCV_Reg->setEnabled(true);
            leCV_Reg->setEnabled(true);
            break;

            // CV 29 bit 2 Value 0
        case progNmra:
            labelCV_Reg->setText(tr("&CV #:"));
            leCV_Reg->setText("29");
            leBit->setText("2");
            leValue->setText("0");
            labelCV_Reg->setEnabled(false);
            leCV_Reg->setEnabled(false);
            labelBit->setEnabled(false);
            leBit->setEnabled(false);
            break;
    }
}

// NMRA dependent programming code
void Programmer::slotProgramNMRA()
{
    int CV_Reg = leCV_Reg->text().toInt();
    int Value = leValue->text().toInt();
    int Bit = leBit->text().toInt();
    unsigned int address = leAddress->text().toUInt();

    PowerMessage pm = PowerMessage(SrcpMessage::mtPowerSet,
            srcpbus, false);
    // stop refresh cycle
    if (bStarted) {
        emit sendSrcpMessage(pm);
        qApp->processEvents();
    }
  
    SmMessage smm = SmMessage(SrcpMessage::mtSmInit, srcpbus,
            SmMessage::smpNmra);
    emit sendSrcpMessage(smm);
    qApp->processEvents();

    switch (actionGroup->checkedId()) {
        case progCv:
            //("GL NMRA CV %d %d", CV_Reg, Value);
            smm = SmMessage(SrcpMessage::mtSmSet, srcpbus, address,
                    SmMessage::smtCv, CV_Reg, Value);
            break;

        case progCvBit:
        case progNmra:
            //("GL NMRA CVBIT %d %d %d", CV_Reg, Bit, Value);
            smm = SmMessage(SrcpMessage::mtSmSet, srcpbus, address,
                    SmMessage::smtCvbit, CV_Reg, Bit, Value);
            break;

        case progReg:
            //("GL NMRA REG %d %d", CV_Reg, Value);
            smm = SmMessage(SrcpMessage::mtSmSet, srcpbus, address,
                    SmMessage::smtReg, CV_Reg, Value);
            break;
    }

    emit sendSrcpMessage(smm);
    qApp->processEvents();

    if (bStarted) {
        pm = PowerMessage(SrcpMessage::mtPowerSet, srcpbus, true);
        emit sendSrcpMessage(pm);
        qApp->processEvents();
    }

    QMessageBox::information(this, "dtcltiny",
                             tr("Programming completed"));
    //accept();
}

// Uhlenbrock dependent programming code
void Programmer::slotProgramUHL()
{
    int CV_Reg = leCV_Reg->text().toInt();
    int Value = leValue->text().toInt();
    int Address = leAddress->text().toInt();

    progress->show();
    qApp->processEvents();

    // activate loco
    ServerMessage sm = ServerMessage(SrcpMessage::mtServerReset);
    emit sendSrcpMessage(sm);
    
    SleeperThread::usleep(20000);

    PowerMessage pm = PowerMessage(SrcpMessage::mtPowerSet,
            srcpbus, true);

    if (!bStarted) {
        emit sendSrcpMessage(pm);
        qApp->processEvents();
    }

    SleeperThread::usleep(20000);
    progress->setValue(5);

    //SET GL <protocol> <addr> <dir> <V> <V_max> <func> <nro_f> <f1> ...
    //args.sprintf("SET GL M4 %d 1 1 0 0 4 0 0 0 0", Address);
    //1. init
    GlMessage glm = GlMessage(SrcpMessage::mtGlInit,
            srcpbus, Address, GlMessage::proMM, 1, 0, 5);
    emit sendSrcpMessage(glm);
    qApp->processEvents();

    if (Address != CV_Reg) {
        // init CV_Reg value to be known by srcp server
        glm = GlMessage(SrcpMessage::mtGlInit,
                srcpbus, CV_Reg, GlMessage::proMM, 1, 0, 5);
        emit sendSrcpMessage(glm);
        qApp->processEvents();
    }

    if (Address != Value && CV_Reg != Value) {
        // init Value value to be known by srcp server
        glm = GlMessage(SrcpMessage::mtGlInit,
                srcpbus, Value, GlMessage::proMM, 1, 0, 5);
        emit sendSrcpMessage(glm);
        qApp->processEvents();
    }

    //2. set
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, Address,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            emit sendSrcpMessage(glm);
            qApp->processEvents();
        }
        SleeperThread::sleep(1);
        progress->setValue(10 + i * 5);
    }

    //SET GL <protocol> <addr> <dir> <V> <V_max> <func> <nro_f> <f1> ...
    //args.sprintf("GL M4 %d 1 0 0 0 4 0 0 0 0", Address);
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, Address,
            5, 1, 0, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();

    for (int i = 0; i < 4; i++) {
        SleeperThread::sleep(1);
        progress->setValue(60 + i * 5);
        qApp->processEvents();
    }

    // activate register
    //args.sprintf("GL M4 %d 1 1 0 0 4 0 0 0 0", CV_Reg);
    // if CV_Reg is different from "Address" and recognized by srcpd
    //as new GL item -> needs init?
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, CV_Reg,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();
    SleeperThread::usleep(20000);

    //args.sprintf("GL M4 %d 1 1 0 0 4 0 0 0 0", CV_Reg);
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, CV_Reg,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();
    SleeperThread::usleep(20000);

    // write data
    //args.sprintf("GL M4 %d 1 1 0 0 4 0 0 0 0", Value);
    //if Value is different from "Address" and recognized by srcpd
    //as new GL item -> needs init?
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, Value,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();
    SleeperThread::usleep(20000);

    //args.sprintf("GL M4 %d 1 0 0 0 4 0 0 0 0", Value);
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, Value,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();
    SleeperThread::usleep(20000);

    // send operation code for quit programming (80)
    //args.sprintf("GL M4 80 1 1 0 0 4 0 0 0 0");
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, 80,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();
    SleeperThread::usleep(20000);

    //args.sprintf("GL M4 80 1 0 0 0 4 0 0 0 0");
    glm = GlMessage(SrcpMessage::mtGlSet, srcpbus, 80,
            5, 1, 1, 0, 0);
    emit sendSrcpMessage(glm);
    qApp->processEvents();

    for (int i = 0; i < 4; i++) {
        SleeperThread::sleep(1);
        progress->setValue(80 + i * 5);
        qApp->processEvents();
    }

    // stop the refresh cycle
    pm = PowerMessage(SrcpMessage::mtPowerSet, srcpbus, false);
    emit sendSrcpMessage(pm);

    if (bStarted) {
        SleeperThread::usleep(20000);
        pm = PowerMessage(SrcpMessage::mtPowerSet, srcpbus, true);
        emit sendSrcpMessage(pm);
    }
    progress->setValue(100);
    qApp->processEvents();
    QMessageBox::information(this, "dtcltiny",
                             tr("Programming completed"));
    //accept();
}
