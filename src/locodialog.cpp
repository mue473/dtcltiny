// locodialog.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
                               LocoDialog.cpp
                             -------------------
    begin                : 11.11.2000
    copyright            : (C) 2000 by Markus Pfeiffer
    email                : mail@markus-pfeiffer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QIntValidator>

#include "locodialog.h"
#include "pixmaps/openfile.xpm"

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    #define SETUTF8 ts.setEncoding(QStringConverter::Utf8)
#else
    #define SETUTF8 ts.setCodec("utf-8");
#endif


LocoDialog::LocoDialog(QWidget* parent): QDialog(parent), maxAddress(0)
{
    setWindowTitle(tr("Change Loco Properties"));
    setModal(true);

    loadDecoderFile();
    loadProtocolFile();

    QVBoxLayout* layoutMain = new QVBoxLayout();
    tabs = new QTabWidget();

    setupTabGeneral();
    setupTabFuncs();
    setupTabSpeeds();
    setupTabCrcf();

    // OK + Cancel button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layoutMain->addWidget(tabs);
    layoutMain->addWidget(buttonBox);
    setLayout(layoutMain);
}

int LocoDialog::exec()
{
    if (decoders.size() > 0 && protocols.size() > 0)
        return QDialog::exec();
    else
        return false;
}

void LocoDialog::setupTabCrcf()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout* baseLayout = new QVBoxLayout(w);

    // name groupbox
    QGroupBox *groupSpeed = new QGroupBox(tr("Speed contol"), w);
    groupSpeed->setObjectName("groupNames");
    baseLayout->addWidget(groupSpeed);

    QVBoxLayout *speedGBLayout = new QVBoxLayout();
    speedGBLayout->setObjectName("speedGBLayout");
    groupSpeed->setLayout(speedGBLayout);

    keepLimitCB = new QCheckBox(tr("&Keep route speed limit"));
    speedGBLayout->addWidget(keepLimitCB);
    keepLimitCB->setToolTip(
            tr("Decrease current speed to speed limit of used route."));

    followLimitCB = new QCheckBox(tr("&Follow route speed limit"));
    speedGBLayout->addWidget(followLimitCB);
    followLimitCB->setToolTip(
            tr("Adjust current speed exactly to speed limit of used route."));

    baseLayout->addStretch();

    tabs->addTab(w, tr("&CRCF"));
}

void LocoDialog::setupTabGeneral()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout* baseLayout = new QVBoxLayout(w);


    // name groupbox
    QGroupBox *groupNames = new QGroupBox(tr("Name and Icon"), w);
    groupNames->setObjectName("groupNames");
    baseLayout->addWidget(groupNames);

    QGridLayout *nameGrid = new QGridLayout();

    QLabel* namelabel = new QLabel(tr("&Name (Loco):"));
    QLabel* aliaslabel = new QLabel(tr("&Train number:"));
    QLabel* iconlabel = new QLabel(tr("&Icon:"));

    leName = new QLineEdit();
    leName->setMaxLength(16);
    namelabel->setBuddy(leName);

    QIntValidator* tv = new QIntValidator(0, 999999, this);
    leAlias = new QLineEdit();
    leAlias->setValidator(tv);
    leAlias->setMaxLength(16);
    aliaslabel->setBuddy(leAlias);

    locoIcon = new QLineEdit();
    locoIcon->setReadOnly(true);
    iconlabel->setBuddy(locoIcon);

    QPushButton* buttGetIcon = new QPushButton();
    buttGetIcon->setIcon(QPixmap(openfile));
    connect(buttGetIcon, SIGNAL(clicked()), this, SLOT(slotGetIcon()));

    nameGrid->addWidget(namelabel, 0 , 0);
    nameGrid->addWidget(aliaslabel, 1 , 0);
    nameGrid->addWidget(iconlabel, 2 , 0);

    nameGrid->addWidget(leName, 0 , 1);
    nameGrid->addWidget(leAlias, 1 , 1);
    nameGrid->addWidget(locoIcon, 2 , 1, 1 , 2);
    //third column must stretch to shrink column two
    nameGrid->setColumnStretch(2, 1);

    nameGrid->addWidget(buttGetIcon, 2 , 3);

    groupNames->setLayout(nameGrid);


    // direction groupbox
    QGroupBox *groupDirection = new QGroupBox(tr("Direction"), w);
    groupDirection->setObjectName("groupDirection");
    baseLayout->addWidget(groupDirection);

    QVBoxLayout *directionGBLayout = new QVBoxLayout();
    directionGBLayout->setObjectName("directionGBLayout");
    groupDirection->setLayout(directionGBLayout);

    cbReverseDirection =
        new QCheckBox(tr("&Reverse Logic (show forward, move backward)"));
    directionGBLayout->addWidget(cbReverseDirection);


    // logic groupbox
    QGroupBox *groupLogic = new QGroupBox(tr("Logic"));
    groupLogic->setObjectName("groupLogic");
    baseLayout->addWidget(groupLogic);

    //line with decoder
    QLabel* decoderlabel = new QLabel(tr("D&ecoder:"));

    decoderCB = new QComboBox();
    decoderCB->setObjectName("decoderCB");
    for (int i = 0; i < decoders.size(); ++i) {
        decoderCB->addItem(decoders.at(i).name, decoders.at(i).functions);
    }
    decoderlabel->setBuddy(decoderCB);

    //line with protocol
    QLabel* protocollabel = new QLabel(tr("&Protocol:"));

    protocolCB = new QComboBox();
    protocolCB->setObjectName("protocolCB");
    protocolCB->setMinimumSize(QSize(80, 0));
    protocollabel->setBuddy(protocolCB);

    //line with address
    QLabel* addresslabel = new QLabel(tr("A&ddress:"));

    spAddress = new QSpinBox(w);
    spAddress->setObjectName("spAddress");
    spAddress->setMinimumSize(QSize(100, 0));
    spAddress->setWrapping(true);
    addresslabel->setBuddy(spAddress);

    //lines with description
    labelDescription = new QLabel("");

    //line with mfxuid
    QLabel* mfxuidlabel = new QLabel(tr("mfx-UID:"));

    leMfxuid = new QLineEdit();
    leMfxuid->setObjectName("leMfxuid");
    leMfxuid->setReadOnly(true);
    mfxuidlabel->setBuddy(leMfxuid);

	// Bind button
	bindPB = new QPushButton(tr("&Bind"), groupLogic);
    connect(bindPB, SIGNAL(clicked()), parent(), SLOT(slotBindRequested()));

    QHBoxLayout *logicLayout = new QHBoxLayout();
    QVBoxLayout *labelLayout = new QVBoxLayout();
    QVBoxLayout *editLayout = new QVBoxLayout();
    QHBoxLayout *decoderLayout = new QHBoxLayout();
    QHBoxLayout *padLayout = new QHBoxLayout();
    QVBoxLayout *paeditLayout = new QVBoxLayout();
    QVBoxLayout *desccontainerLayout = new QVBoxLayout();
    QHBoxLayout *descriptionLayout = new QHBoxLayout();
    QVBoxLayout *bindLayout = new QVBoxLayout();

    labelLayout->addWidget(decoderlabel);
    labelLayout->addWidget(protocollabel);
    labelLayout->addWidget(addresslabel);
    labelLayout->addWidget(mfxuidlabel);
    logicLayout->addLayout(labelLayout);

    decoderLayout->addWidget(decoderCB);
    decoderLayout->addStretch();
    editLayout->addLayout(decoderLayout);

    paeditLayout->addWidget(protocolCB);
    paeditLayout->addWidget(spAddress);
    paeditLayout->addWidget(leMfxuid);
    padLayout->addLayout(paeditLayout);

    descriptionLayout->addWidget(labelDescription);
    descriptionLayout->addStretch();
    desccontainerLayout->addLayout(descriptionLayout);

    desccontainerLayout->addStretch();
    padLayout->addLayout(desccontainerLayout);

    editLayout->addLayout(padLayout);
    logicLayout->addLayout(editLayout);

    bindLayout->addStretch();
    bindLayout->addWidget(bindPB);
    logicLayout->addLayout(bindLayout);

    groupLogic->setLayout(logicLayout);

    // update protocol checkbox if new decoder is selected
    connect(decoderCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateProtocolCB(int)));

    //change despription if protocol is highlighted
    connect(protocolCB, SIGNAL(highlighted(int)),
            this, SLOT(updateProtocolDescription(int)));

    //change function list and slider parameters if protocol is selected
    connect(protocolCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateProtocolData(int)));

    // finally insert page one into the tab dialog
    tabs->addTab(w, tr("&General"));
}


void LocoDialog::setupTabFuncs()
{
    QScrollArea *sa = new QScrollArea(this);

    QWidget *functionArea = new QWidget();
    functionLayout = new QVBoxLayout();
    functionLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    sa->setWidget(functionArea);
    sa->setWidgetResizable(true);
    functionArea->setLayout(functionLayout);

    // functions are inserted by updateFunctionView() when dialog
    // parameter for functions is set

    // finally insert page one into the tab dialog
    tabs->addTab(sa, tr("&Functions"));
}


// set up page three of the tab dialog
void LocoDialog::setupTabSpeeds()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout* baseLayout = new QVBoxLayout();

    QGroupBox *groupStatic = new QGroupBox(tr("Static"));
    groupStatic->setObjectName("groupStatic");

    QVBoxLayout* staticLayout = new QVBoxLayout();
    groupStatic->setLayout(staticLayout);

    QHBoxLayout* unitLayout = new QHBoxLayout();

    //
    QLabel *max = new QLabel(tr("Maximum &value complies speed value"));

    leRealMaxSpeed = new QLineEdit();
    leRealMaxSpeed->setMaximumWidth(30);
    leRealMaxSpeed->setMaxLength(3);
    max->setBuddy(leRealMaxSpeed);

    //
    QButtonGroup *groupUnits = new QButtonGroup(groupStatic);

    rbUnitkm = new QRadioButton("km/h");
    groupUnits->addButton(rbUnitkm, 0);

    QRadioButton* rbUnitml = new QRadioButton("mph");
    groupUnits->addButton(rbUnitml, 1);


    unitLayout->addWidget(max);
    unitLayout->addWidget(leRealMaxSpeed);
    unitLayout->addWidget(rbUnitkm);
    unitLayout->addWidget(rbUnitml);
    unitLayout->addStretch();
    staticLayout->addLayout(unitLayout);

    // Grid layout for three sliders and led indicators
    QGridLayout* staticGrid = new QGridLayout();

    // setup sliders
    sliderMaxSpeed =
        new QSlider(Qt::Horizontal);
    sliderMaxSpeed->setTickPosition(QSlider::TicksBelow);
    sliderMaxSpeed->setMinimumWidth(300);

    sliderAvgSpeed =
        new QSlider(Qt::Horizontal);
    sliderAvgSpeed->setTickPosition(QSlider::TicksBelow);

    sliderMinSpeed =
        new QSlider(Qt::Horizontal);
    sliderMinSpeed->setTickPosition(QSlider::TicksBelow);

    staticGrid->addWidget(sliderMaxSpeed, 0, 1);
    staticGrid->addWidget(sliderAvgSpeed, 1, 1);
    staticGrid->addWidget(sliderMinSpeed, 2, 1);

    // setup labels after sliders to handle buddy
    QLabel* maxlabel = new QLabel(tr("&Maximum:"));
    maxlabel->setBuddy(sliderMaxSpeed);
    maxlabel->setMinimumWidth(90);

    QLabel* avlabel = new QLabel(tr("&Average:"));
    avlabel->setBuddy(sliderMaxSpeed);

    QLabel* minlabel = new QLabel(tr("M&inimum:"));
    minlabel->setBuddy(sliderMaxSpeed);

    staticGrid->addWidget(maxlabel, 0, 0);
    staticGrid->addWidget(avlabel, 1, 0);
    staticGrid->addWidget(minlabel, 2, 0);

    // setup digital displays
    QLCDNumber *maxlcdNum = new QLCDNumber(3);
    maxlcdNum->setMinimumWidth(60);
    maxlcdNum->setMinimumHeight(32);
    maxlcdNum->setFrameStyle(QFrame::NoFrame);
    maxlcdNum->setSegmentStyle(QLCDNumber::Filled);
    connect(sliderMaxSpeed, SIGNAL(valueChanged(int)), maxlcdNum,
            SLOT(display(int)));

    QLCDNumber *avlcdNum = new QLCDNumber(3);
    avlcdNum->setMinimumHeight(32);
    avlcdNum->setFrameStyle(QFrame::NoFrame);
    avlcdNum->setSegmentStyle(QLCDNumber::Filled);
    connect(sliderAvgSpeed, SIGNAL(valueChanged(int)), avlcdNum,
            SLOT(display(int)));

    QLCDNumber *minlcdNum = new QLCDNumber(3);
    minlcdNum->setMinimumHeight(32);
    minlcdNum->setFrameStyle(QFrame::NoFrame);
    minlcdNum->setSegmentStyle(QLCDNumber::Filled);
    connect(sliderMinSpeed, SIGNAL(valueChanged(int)), minlcdNum,
            SLOT(display(int)));

    staticGrid->addWidget(maxlcdNum, 0, 2);
    staticGrid->addWidget(avlcdNum, 1, 2);
    staticGrid->addWidget(minlcdNum, 2, 2);

    staticLayout->addLayout(staticGrid);


    // Grid layout for two sliders and led indicators
    QGridLayout* dynamicGrid = new QGridLayout();

    QGroupBox *groupDynamic = new QGroupBox(tr("Dynamic"));
    groupDynamic->setObjectName("groupDynamic");

    sliderAccelTime = new QSlider(Qt::Horizontal);
    sliderAccelTime->setMinimumWidth(300);
    sliderAccelTime->setRange(0, 1500);
    sliderAccelTime->setTickPosition(QSlider::TicksBelow);
    sliderAccelTime->setTickInterval(100);
    sliderAccelTime->setSingleStep(50);
    sliderAccelTime->setPageStep(50);

    sliderBreakTime = new QSlider(Qt::Horizontal);
    sliderBreakTime->setRange(0, 1500); // max 1500 ms = 1.5 secs
    sliderBreakTime->setTickPosition(QSlider::TicksBelow);
    sliderBreakTime->setTickInterval(100);
    sliderBreakTime->setSingleStep(50);
    sliderBreakTime->setPageStep(50);

    QLabel *acclabel = new QLabel(tr("A&cceleration:"));
    acclabel->setBuddy(sliderAccelTime);
    acclabel->setMinimumWidth(90);

    QLabel *breaklabel = new QLabel(tr("&Breaking:"));
    breaklabel->setBuddy(sliderBreakTime);


    QLCDNumber* acclcdNum = new QLCDNumber(4);
    acclcdNum->setMinimumWidth(60);
    acclcdNum->setMinimumHeight(32);
    acclcdNum->setFrameStyle(QFrame::NoFrame);
    acclcdNum->setSegmentStyle(QLCDNumber::Filled);
    connect(sliderAccelTime, SIGNAL(valueChanged(int)), acclcdNum,
            SLOT(display(int)));

    QLCDNumber* breaklcdNum = new QLCDNumber(4);
    breaklcdNum->setMinimumHeight(32);
    breaklcdNum->setFrameStyle(QFrame::NoFrame);
    breaklcdNum->setSegmentStyle(QLCDNumber::Filled);
    connect(sliderBreakTime, SIGNAL(valueChanged(int)), breaklcdNum,
            SLOT(display(int)));


    dynamicGrid->addWidget(acclabel, 0, 0);
    dynamicGrid->addWidget(breaklabel, 1, 0);
    dynamicGrid->addWidget(sliderAccelTime, 0, 1);
    dynamicGrid->addWidget(sliderBreakTime, 1, 1);
    dynamicGrid->addWidget(acclcdNum, 0, 2);
    dynamicGrid->addWidget(breaklcdNum, 1, 2);
    groupDynamic->setLayout(dynamicGrid);


    baseLayout->addWidget(groupStatic);
    baseLayout->addWidget(groupDynamic);
    baseLayout->addStretch();
    w->setLayout(baseLayout);

    // finally insert page one into the tab dialog
    tabs->addTab(w, tr("&Speeds"));
}


void LocoDialog::verify()
{
    if ((QString(spAddress->text()).toInt() > maxAddress)
        || (QString(spAddress->text()).toInt() < 1)) {
        QMessageBox::information(this, tr("Error"),
            tr("Address out of range. Please enter\n"
               "a correct value from 1 to %1.").arg(maxAddress));
        return;
    }

    accept();
}


void LocoDialog::slotGetIcon()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open locomotive image"),
                LOCO_PIX_DIR, "Pixmaps (*.xpm *.png)", NULL,
				QFileDialog::DontUseNativeDialog);
    if (!filename.isEmpty()) {
        QFileInfo file(filename);
        if (!file.exists()) {
            QMessageBox::warning(this, tr("Error"),
                    tr("Sorry, this file doesn't exist.\n%1").
                    arg(filename));
            return;
        }
        QFileInfo pixdir(QString(LOCO_PIX_DIR));
#ifdef WIN32
        if (file.absolutePath() != pixdir.absoluteFilePath()) {
#else
        if (file.absolutePath() != pixdir.path()) {
#endif
            QMessageBox::warning(this, tr("Error"),
                    tr("Sorry, directory \'%1\' is no valid pixmap path.\n"
                        "Only files in directory \'%2\' are valid.")
                    .arg(file.filePath())
                    .arg(pixdir.absoluteFilePath()));
            return;
        }
        // remove path
        filename = file.fileName();
        locoIcon->setText(filename);
    }
}


/*Update protocol parameters if protocol is selected*/
void LocoDialog::updateProtocolData(int protid)
{
    bool found = false;
	QString prot = protocolCB->itemText(protid);

    for (int i = 0; i < protocols.size(); ++i) {
        if (protocols.at(i).name == prot) {
            updateProtocolValues(protocols.at(i));
            found = true;
            break;
        }
    }
    if (!found) {
        QMessageBox::critical(this, tr("Error"),
                tr("Sorry, protocol \"%1\" is not defined.\n"
                    "Please check the protocols and decoders files.")
                .arg(prot));
    }
}


/*Update protocol description test when protocol gets highlighted*/
void LocoDialog::updateProtocolDescription(int protid)
{
    bool found = false;
	QString prot = protocolCB->itemText(protid);

    for (int i = 0; i < protocols.size(); ++i) {
        if (protocols.at(i).name == prot) {
            printDescription(protocols.at(i));
            found = true;
            break;
        }
    }
    if (!found) {
        QMessageBox::critical(this, tr("Error"),
                tr("Sorry, protocol \"%1\" is not defined.\n"
                    "Please check the protocols and decoders files.")
                .arg(prot));
    }
    else {
        leMfxuid->setEnabled(prot[0] == 'X');
    	bindPB->setEnabled(prot[0] == 'X');
	}
}


void LocoDialog::updateProtocolValues(const Protocol& prot)
{
    spAddress->setRange(1, prot.addressrange);
    sliderMaxSpeed->setRange(0, prot.steps);
    sliderAvgSpeed->setRange(0, prot.steps);
    sliderMinSpeed->setRange(0, prot.steps);

    sliderMaxSpeed->setValue(sliderMaxSpeed->maximum());
    sliderAvgSpeed->setValue(sliderMaxSpeed->maximum() / 2);
    sliderMinSpeed->setValue(sliderMaxSpeed->maximum() / 3);

    sliderMaxSpeed->setTickInterval(prot.steps / 10);
    sliderAvgSpeed->setTickInterval(prot.steps / 10);
    sliderMinSpeed->setTickInterval(prot.steps / 10);

    //This has been moved to decoder selection change as the number of
    //available functions is a matter of decoder not of protocol:
    //updateFunctionView(prot.functions);
}


bool LocoDialog::loadDecoderFile()
{
    QFile file(DECODER_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream ts(&file);
        SETUTF8;
        QString line, name;
        unsigned int functions = 1;

        while (!ts.atEnd()) {
            line = ts.readLine();

            // ignore lines beginning with '#'
            if (line.startsWith("#") || line.isEmpty())
                    continue;

            QStringList tokens = line.split('=');

            if (tokens[0] =="NAME") {
                name = tokens[1];
            }
            else if (tokens[0] == "FUNCTIONS") {
                functions = tokens[1].toUInt();
            }
            else if (tokens[0] == "PROTOCOLS") {
                decoders.append(Decoder(name, functions, tokens[1]));
            }
        }
        return true;
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                tr("Decoder file '%1' not found.").arg(DECODER_FILE));
        return false;
    }
}


bool LocoDialog::loadProtocolFile()
{
    QFile file(PROTOCOL_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream ts(&file);
        SETUTF8;
        QString line, name;
        unsigned int steps = 0;

        while (!ts.atEnd()) {
            line = ts.readLine();

            // ignore lines beginning with '#'
            if (line.startsWith("#") || line.isEmpty())
                    continue;

            QStringList tokens = line.split('=');

            if (tokens[0] == "NAME") {
                name = tokens[1];
            }
            else if (tokens[0] == "STEPS") {
                steps = tokens[1].toUInt();
            }
            else if (tokens[0] == "ADDRESSRANGE") {
                protocols.append(Protocol(name, steps,
                                tokens[1].toUInt()));
            }
        }
        return true;
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                tr("Protocol file '%1' not found.").arg(PROTOCOL_FILE));
        return false;
    }
}


void LocoDialog::updateProtocolCB(int decoderid)
{
    if (decoderid < 0) return;

    //prevent currentIndexChanged() signal by clear()
    disconnect(protocolCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateProtocolData(int)));

    QStringList protolist = decoders.at(decoderid).protocols.split(' ');
    if (!protolist.isEmpty()) {
        protocolCB->clear();
        protocolCB->addItems(protolist);
        updateProtocolDescription(protocolCB->currentIndex());
    }
    else
        qWarning("Error: No protocols for decoder %d found.", decoderid);

    connect(protocolCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateProtocolData(int)));

    //update function list on second tab
    updateFunctionView(decoders.at(decoderid).functions);
}


void LocoDialog::printDescription(const Protocol& prot)
{
    unsigned int fc = decoderCB->itemData(decoderCB->currentIndex()).toUInt();

    QString description=
            tr("The selected decoder supports %1 functions.\n"
                    "This protocol supports %2 speed steps and\n"
                    "%3 addresses.")
            .arg(fc)
            .arg(prot.steps)
            .arg(prot.addressrange);

    labelDescription->setText(description);
    maxAddress = prot.addressrange;
}


void LocoDialog::setLocoName(const QString& ln)
{
    leName->setText(ln);
}


QString LocoDialog::getLocoName() const
{
    return leName->text();
}


void LocoDialog::setTrainNumber(unsigned int tn)
{
    leAlias->setText(QString::number(tn));
}


unsigned int LocoDialog::getTrainNumber()
{
    return leAlias->text().toUInt();
}


void LocoDialog::setLocoPixmapName(const QString& pn)
{
    locoIcon->setText(pn);
}


QString LocoDialog::getLocoPixmapName() const
{
    return locoIcon->text();
}


void LocoDialog::setReverseDirection(bool rd)
{
    cbReverseDirection->setChecked(rd);
}


bool LocoDialog::getReverseDirection()
{
    return cbReverseDirection->isChecked();
}


void LocoDialog::setDecoder(const QString& dec, const QString& prot)
{
    bool found = false;

    for (int i = 0; i < decoderCB->count(); i++) {
        if (decoderCB->itemText(i) == dec) {
            decoderCB->setCurrentIndex(i);
            found = true;
        }
    }
    if (!found) {
        updateProtocolCB(decoderCB->currentIndex());
        QMessageBox::warning(this, tr("Error"),
                             tr("Decoder \"%1\" not recognized.\n"
                                 "Check to choose a known one\n"
                                 "and save your new configuration.")
                             .arg(dec));
        return;
    }

    if (protocolCB->count() > 0)
        for (int i = 0; i < protocolCB->count(); i++) {
            if (protocolCB->itemText(i) == prot) {
                protocolCB->setCurrentIndex(i);
                updateProtocolDescription(i);
                updateProtocolData(i);
                break;
            }
        }
    else {
        QMessageBox::warning(this, tr("Error"),
                             tr("No protocols found for decoder \"%1\".\n"
                                 "Select a listed decoder and\n"
                                 "choose an available protocol.")
                             .arg(dec));
    }
}


void LocoDialog::getDecoder(QString& dec, QString& prot)
{
    dec = decoderCB->currentText();
    prot = protocolCB->currentText();
}


void LocoDialog::setAddress(int addr)
{
    spAddress->setValue(addr);
}


int LocoDialog::getAddress()
{
    return spAddress->text().toInt();
}


void LocoDialog::setMfxUid(unsigned int uid)
{
	if (uid) leMfxuid->setText(QString("%1").arg(uid, 8, 16).toUpper());
}


void LocoDialog::setAttenuationTimes(int acc, int bt)
{
    sliderAccelTime->setValue(acc);
    sliderBreakTime->setValue(bt);
}


void LocoDialog::getAttenuationTimes(int& acc, int& bt)
{
    acc = sliderAccelTime->value();
    bt=  sliderBreakTime->value();
}


void LocoDialog::setLocoSpeeds(int max, int avg, int min, int rms)
{
    sliderMaxSpeed->setValue(max);
    sliderAvgSpeed->setValue(avg);
    sliderMinSpeed->setValue(min);
    leRealMaxSpeed->setText(QString::number(rms));
}


void LocoDialog::getLocoSpeeds(int& max, int& avg, int& min, int& rms)
{
    max = sliderMaxSpeed->value();
    avg = sliderAvgSpeed->value();
    min = sliderMinSpeed->value();
    rms = leRealMaxSpeed->text().toInt();
}


void LocoDialog::setSpeedUnitKmh(bool iskmh)
{
    rbUnitkm->setChecked(iskmh);
	((rbUnitkm->group())->button(1))->setChecked(!iskmh);
}


bool LocoDialog::getSpeedUnitKmh()
{
    return rbUnitkm->isChecked();
}

void LocoDialog::setKeepLimit(bool kl)
{
    keepLimitCB->setChecked(kl);
}


bool LocoDialog::getKeepLimit()
{
    return keepLimitCB->isChecked();
}

void LocoDialog::setFollowLimit(bool kl)
{
    followLimitCB->setChecked(kl);
}


bool LocoDialog::getFollowLimit()
{
    return followLimitCB->isChecked();
}


/*edit function list*/
void LocoDialog::setFunctions(const QList<Function>& fl)
{
    updateFunctionView(fl.size());

    for (int i = 0; i < fl.size(); ++i) {
        functionwidgets.at(i).buttontypeCB->setCurrentIndex(
                !fl[i].buttontype);
        functionwidgets.at(i).functiontypeCB->setCurrentIndex(
                fl[i].functiontype);
    }
}

/* return updated function list */
void LocoDialog::getFunctions(QList<Function>& fl)
{
    //first check size of both lists and adapt function list
    if (functionwidgets.size() > fl.size()) {
        while (functionwidgets.size() > fl.size()) {
            //append function
            fl << Function(0, PUSHBUTTON, false);
        }
    }
    else if (functionwidgets.size() < fl.size()) {
        while (functionwidgets.size() < fl.size()) {
            //remove function
            fl.takeLast();
        }
    }

    for (int i = 0; i < functionwidgets.size(); i++) {
        fl[i].active = false;
        fl[i].buttontype = !functionwidgets.at(i).buttontypeCB->currentIndex();
        fl[i].functiontype = functionwidgets.at(i).functiontypeCB->currentIndex();
    }
}

/* add a new line to function view*/
void LocoDialog::addFunctionLine(int index)
{
    FunctionWidgets fw = FunctionWidgets(NULL, NULL, NULL, NULL,	// NULL,
            NULL, NULL);

    fw.lineLayout = new QHBoxLayout();
    fw.lineLayout->setSpacing(22);

    fw.fLabel = new QLabel(tr("F%1").arg(index));

    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->setSpacing(8);

    fw.typeLabel = new QLabel(tr("&Type:"));
    fw.functiontypeCB = new QComboBox();
    fw.typeLabel->setBuddy(fw.functiontypeCB);

    // add preconfigured items
    for (int i = 0; i < functiontypes ; i++) {
        fw.functiontypeCB->addItem(tr(ftData[i].ftName));
    }

    typeLayout->addWidget(fw.typeLabel);
    typeLayout->addWidget(fw.functiontypeCB);


    QHBoxLayout *activateLayout = new QHBoxLayout();
    activateLayout->setSpacing(8);

    fw.activateLabel = new QLabel(tr("&Activate as:"));
    fw.buttontypeCB = new QComboBox();
    fw.buttontypeCB->addItem(tr("Switch"));
    fw.buttontypeCB->addItem(tr("Pushbutton"));
    fw.activateLabel->setBuddy(fw.buttontypeCB);

    activateLayout->addWidget(fw.activateLabel);
    activateLayout->addWidget(fw.buttontypeCB);

    fw.lineLayout->addWidget(fw.fLabel);
//    fw.lineLayout->addStretch();
//    fw.lineLayout->addWidget(fw.visibleCB);
    fw.lineLayout->addLayout(typeLayout);
    fw.lineLayout->addLayout(activateLayout);

    functionLayout->addLayout(fw.lineLayout);

    functionwidgets << fw;
}

/* remove obsolete line from function view*/
void LocoDialog::removeFunctionLine()
{
    FunctionWidgets *fw = &functionwidgets.last();

    // uncheck box to keep visibility limit up to date
//    if (fw->visibleCB->isChecked())
//        fw->visibleCB->click();

    //remove complete function line
    delete fw->fLabel;
//    delete fw->visibleCB;
    delete fw->typeLabel;
    delete fw->functiontypeCB;
    delete fw->activateLabel;
    delete fw->buttontypeCB;
    delete fw->lineLayout;
    functionwidgets.removeLast();
}

/*update number of editable functions on function tab*/
void LocoDialog::updateFunctionView(int fcount)
{
	if (fcount > maxfbuttoncount) fcount = maxfbuttoncount;

    // nothing to do if number does not change
    if (fcount == functionwidgets.size() || fcount < 0)
        return;

    // add additional lines
    if (fcount > functionwidgets.size()) {
        int missing = fcount - functionwidgets.size();
        for (int i = 0; i < missing; i++)
            addFunctionLine(functionwidgets.size());
    }

    // remove obsolete lines
    else {
        int obsolete = functionwidgets.size() - fcount;
        for (int i = 0; i < obsolete; i++)
            removeFunctionLine();
    }
}
