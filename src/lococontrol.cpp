// lococontrol.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
                               lococontrol.cpp
                             -------------------
 Begin        : 11.11.2000
 Last modified: $Date: 2008/11/05 08:42:40 $
 Copyright    : (C) 2000 by Markus Pfeiffer
                (C) 2008 by Guido Scholz
 E-Mail       : guido-scholz@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QDate>
#include <QtDebug>

#include "crcfmessage.h"
#include "lococontrol.h"
#include "mainwindow.h"

#include "pixmaps/brake.xpm"
#include "pixmaps/fdirection.xpm"
#include "pixmaps/bdirection.xpm"
#include "pixmaps/emergency.xpm"
#include "pixmaps/lock.xpm"

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    #define endl Qt::endl
    #define CLICKED  SIGNAL(idClicked(int))
    #define PRESSED  SIGNAL(idPressed(int))
    #define RELEASED SIGNAL(idReleased(int))
#else
    #define CLICKED  SIGNAL(buttonClicked(int))
    #define PRESSED  SIGNAL(buttonPressed(int))
    #define RELEASED SIGNAL(buttonReleased(int))
#endif

#define OPER_INTERVAL 3

/*string constant key values for data file*/
static const char DF_ADDRESS[] =      "address";
static const char DF_NAME[] =         "name(loco)";
static const char DF_ALIAS[] =        "alias(train)" ;
static const char DF_SHOWWHAT[]=      "show what";
static const char DF_ICON[] =         "icon";
static const char DF_PROTOCOL[] =     "protocol";
static const char DF_DECODER[] =      "decoder";
static const char DF_MFXUID[] =       "mfxuid";
static const char DF_DIRECTION[] =    "direction";
static const char DF_REVDIRECTION[] = "revdirection";
static const char DF_SHOWSPEED[] =    "show speed";
static const char DF_SPEEDUNIT[] =    "speed unit";
static const char DF_REALMAXSPEED[] = "real max speed";
static const char DF_MINSPEED[] =     "min_speed";
static const char DF_AVGSPEED[] =     "avg_speed";
static const char DF_MAXSPEED[] =     "max_speed";
static const char DF_ACCELTIME[] =    "accel_time";
static const char DF_BREAKTIME[] =    "break_time";
static const char DF_KEEPLIMIT[] =    "keeplimit";
static const char DF_FOLLOWLIMIT[] =  "followlimit";
static const char DF_RVNAME[] =       "rvshowname";
static const char DF_RVENTRYASPECT[] ="rventryaspect";
static const char DF_RVEXITASPECT[] = "rvexitaspect";
static const char DF_RVLENGTH[] =     "rvlength";
static const char DF_RVSPEED[] =      "rvspeed";
static const char DF_RVLIMITROUTES[] ="rvlimitroutes";
static const char DF_RVROUTELIMIT[] = "rvroutelimit";
static const char DF_OPERTIME[] =	  "opertime";
static const char DF_LASTUSED[] =	  "lastused";

// new functions
static const char DF_F_STATE[] =      "f_state";
static const char DF_F_FTYPE[] =      "f_ftype";
static const char DF_F_BTYPE[] =      "f_btype";

static const char CFONT[] = "helvetica";

enum {
    C_WIDTH_ = 155,
    C_HEIGHT_ = 450
};

enum {
    SPEED_HALT = 0,
    SPEED_MIN = 1,
    SPEED_MID = 2,
    SPEED_MAX = 3
};

enum showID
{
    NAME,
    ALIAS
};

enum trainDirection {tdirNone = 0, tdirBackward, tdirForward,
    tdirEmergency, tdirToggleDir};

/*
 * constructor to load loco data from text stream
 */
LocoControl::LocoControl(QTextStream& ts, int cid, unsigned int sb,
        QWidget* parent):
    QFrame(parent),
    id(cid),
    modified(false),
    srcpstate(stateNone),
    srcpbus(sb),
    commandsessionid(0),
    infosessionid(0),
    gmreplyid(0),
    currentroute(0),
    nextroute(0),
    keeplimit(true),
    followlimit(false),
    scale(4),
    scaledivisor(87.0),
    keypressed(false)
{
    setObjectName("LocoControl");
    setBackgroundRole(QPalette::Button);
    setAutoFillBackground(true);
    setDefaultValues();
    readFileTextFromStream(ts);

    initLayout();
    updateDisplay();
    updateSpeedButtonText();
}


/*
 * constructor to create a new loco with default values
 */
LocoControl::LocoControl(int cid, unsigned int sb, QWidget* parent):
    QFrame(parent),
    id(cid),
    modified(false),
    srcpstate(stateNone),
    srcpbus(sb),
    commandsessionid(0),
    infosessionid(0),
    gmreplyid(0),
    currentroute(0),
    nextroute(0),
    keeplimit(true),
    followlimit(false),
    scale(4),
    scaledivisor(87.0),
    keypressed(false)
{
    setObjectName("LocoControl");
    setDefaultValues();
    functions << Function(0, SWITCH, false);

    initLayout();
    updateDisplay();
    updateSpeedButtonText();
}


void LocoControl::setDefaultValues()
{
    rvo.showname = true;
    rvo.showentryaspect = true;
    rvo.showexitaspect = true;
    rvo.showlength = true;
    rvo.showspeed = true;
    rvo.limitroutes = false;
    rvo.routelimit = 3;
    directionChanged = false;
    reverseDirection = false;
    powerOn = false;
    currentSpeedStep = 0;
    previousSpeedStep = currentSpeedStep;
    targetSpeedStep = 0;

    // saveable values
    address = 1;
    locoName = "Name";
    trainnumber = 0;
    showloconame = NAME;
    locoPicture = "";
    locoProtocol = "N1";
    decoderSpeedSteps = getDecoderSpeedSteps(locoProtocol);
    locoDecoder = "Generic NMRA DCC";
    mfxUid = 0;
    today = QDate::currentDate().QDate::toString("yy-MM-dd");
    lastUsed = "00-00-00";
    operationTime = 0;
    drivemode = 1;
    iLocoShowSpeed = 1;
    locoSpeedUnit = "km/h";
    maximumSpeed = 100;
    minSpeedStep = decoderSpeedSteps/3;
    avgSpeedStep = decoderSpeedSteps/2;
    maxSpeedStep = decoderSpeedSteps;
    // for more precise calculations this factor should be kept in a
    // lookup table with separate values for each speedstep
    // TODO: add speed calibration
    fLocoSpeedFactor = (float) maximumSpeed / maxSpeedStep;
    iLocoAccelTime = 1000;
    iLocoBreakTime = 700;
    glmfmap = 0;
    routesWindow = NULL;
}


void LocoControl::initLayout()
{
    setFixedSize(C_WIDTH_, C_HEIGHT_);
    setLineWidth(1);
    setFrameStyle(QFrame::Panel | QFrame::Raised);

    /*setup context menu*/
    contextmenu = new QMenu(this);

    QAction *propertiesAction = new QAction(tr("&Properties..."),
            contextmenu);
    connect(propertiesAction, SIGNAL(triggered()),
            this, SLOT(runPropertiesDialog()));
    contextmenu->addAction(propertiesAction);

    QAction *hideAction = new QAction(tr("&Hide"), contextmenu);
    connect(hideAction, SIGNAL(triggered()),
            this, SLOT(hide()));
    contextmenu->addAction(hideAction);

    lockAction = new QAction(tr("&Lock"), contextmenu);
    connect(lockAction, SIGNAL(triggered()),
            this, SLOT(toggleControllerLock()));
    contextmenu->addAction(lockAction);
    lockAction->setEnabled(false);

    contextmenu->addSeparator();

    QAction *showRoutesAction = new QAction(tr("&Show routes..."),
            contextmenu);
    connect(showRoutesAction, SIGNAL(triggered()),
            this, SLOT(showRoutes()));
    contextmenu->addAction(showRoutesAction);

    QAction *clearRoutesAction = new QAction(tr("&Clear routes"),
            contextmenu);
    connect(clearRoutesAction, SIGNAL(triggered()),
            this, SLOT(clearRoutes()));
    contextmenu->addAction(clearRoutesAction);

    QAction *routesViewOptionsAction = new QAction(
            tr("&Routes view options..."), contextmenu);
    connect(routesViewOptionsAction, SIGNAL(triggered()),
            this, SLOT(editRoutesViewOptions()));
    contextmenu->addAction(routesViewOptionsAction);

    contextmenu->addSeparator();

    QAction *deleteAction = new QAction(tr("&Delete"), contextmenu);
    connect(deleteAction, SIGNAL(triggered()),
            this, SLOT(slotDelete()));
    contextmenu->addAction(deleteAction);

    delayTimer = new QTimer(this);
    connect(delayTimer, SIGNAL(timeout()), SLOT(slotSpeedTick()));

    operationTimer = new QTimer(this);
    connect(operationTimer, SIGNAL(timeout()), SLOT(slotOperTick()));
    operationTimer->start(OPER_INTERVAL * 1000);

    haltTimer = new QTimer(this);
    haltTimer->setSingleShot(true);
    connect(haltTimer, SIGNAL(timeout()), SLOT(executeHalt()));

    // basic layout for two vertical frames
    QVBoxLayout* baseLayout = new QVBoxLayout();
    baseLayout->setObjectName("baseLayout");
    baseLayout->setSpacing(3);
    baseLayout->setContentsMargins(3, 3, 3, 3);
    setLayout(baseLayout);

    /* I top section*/
    //frame for speed and function controll
    QFrame* frameSpeed = new QFrame();
    frameSpeed->setFrameStyle(QFrame::Box | QFrame::Sunken);
    baseLayout->addWidget(frameSpeed);

    // vertical layout for three speed sections
    QVBoxLayout* vspeedLayout = new QVBoxLayout();
    vspeedLayout->setObjectName("vspeedLayout");
    vspeedLayout->setSpacing(0);
    vspeedLayout->setContentsMargins(4, 4, 4, 4);
    frameSpeed->setLayout(vspeedLayout);

    // horizontal layout for three items in speed section
    QHBoxLayout* hspeedLayout = new QHBoxLayout();
    hspeedLayout->setObjectName("hspeedLayout");
    hspeedLayout->setSpacing(7);
    hspeedLayout->setContentsMargins(4, 4, 4, 4);
    vspeedLayout->addLayout(hspeedLayout);

    /*1. Left slider for speed control, height 200 pixels*/
    speedSlider = new QSlider(Qt::Vertical);
    hspeedLayout->addWidget(speedSlider);
    speedSlider->setRange(0, maxSpeedStep);
    // for display only, speed is set to 0 by maxSpeedStep -
    // valueChanged in slotUpdateSpeed(int)
    speedSlider->setTickPosition(QSlider::TicksLeft);
    speedSlider->setTickInterval(maxSpeedStep / 10);
    // accept keyboard and mouseclick to activate
    speedSlider->setFocusPolicy(Qt::StrongFocus);
    speedSlider->setToolTip(tr("Change speed"));

    if (maxSpeedStep >= 28)
        speedSlider->setPageStep(maxSpeedStep / 14);
    else
        speedSlider->setPageStep(2);

    connect(speedSlider, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateSpeed(int)));
    speedSlider->setFocus();
    float fFaktor = 150.0 / (float) maxSpeedStep;

    // vertical layout for slim speed buttons
    QVBoxLayout* speedButtonLayout = new QVBoxLayout();
    speedButtonLayout->setObjectName("vspeedLayout");
    speedButtonLayout->setSpacing(0);
//    speedButtonLayout->setContentsMargins(4, 4, 4, 4);

    m_group = new QButtonGroup(this);

    /* 2. colored speed bars*/
    m1 = new QPushButton();
    m1->setStyleSheet("color: white; background-color: green; border: none");
    m1->setMaximumWidth(9);
    m1->setMinimumHeight(25 + qRound(fFaktor * minSpeedStep));
    m_group->addButton(m1, SPEED_MIN);
    m1->setFocusPolicy(Qt::NoFocus);
    m1->setToolTip(tr("Change speed to minimum"));

    m2 = new QPushButton();
    m2->setStyleSheet("background-color: yellow; border: none");
    m2->setMaximumWidth(9);
    m2->setMinimumHeight(25 + qRound(fFaktor * (avgSpeedStep - minSpeedStep)));
    m_group->addButton(m2, SPEED_MID);
    m2->setFocusPolicy(Qt::NoFocus);
    m2->setToolTip(tr("Change speed to average"));

    m3 = new QPushButton();
    m3->setStyleSheet("color: white; background-color: red; border: none");
    m3->setMaximumWidth(9);
    m3->setMinimumHeight(25 + qRound(fFaktor * (maxSpeedStep - avgSpeedStep)));
    m_group->addButton(m3, SPEED_MAX);
    m3->setFocusPolicy(Qt::NoFocus);
    m3->setToolTip(tr("Change speed to maximum"));
    connect(m_group, CLICKED, SLOT(slotSetAutoSpeed(int)));

    speedButtonLayout->addWidget(m3);
    speedButtonLayout->addWidget(m2);
    speedButtonLayout->addWidget(m1);
    hspeedLayout->addLayout(speedButtonLayout);

    /* 3. series of function buttons */
    QVBoxLayout* functionLayout = new QVBoxLayout();
    functionLayout->setObjectName("functionLayout");
    functionLayout->setSpacing(0);

    groupFuncs = new QButtonGroup();
    groupFuncs->setExclusive(false);

    // add standard function buttons
    for (int i = 0; i < maxfbuttoncount; i++) {
        buttF[i] = new QPushButton();
        buttF[i]->setMinimumSize(36, 36);
        buttF[i]->setMaximumSize(36, 36);
        buttF[i]->setIconSize(QSize(30, 30));
        buttF[i]->setFocusPolicy(Qt::NoFocus);
        functionLayout->addWidget(buttF[i]);
        groupFuncs->addButton(buttF[i], -1);
    }

    //map visible functions to user interface buttons
    int buttons = 0;
    for (int i = 0; i < functions.size(); ++i) {
        if (functions.at(i).functiontype > 0) {
            setFuButton(buttF[buttons], functions.at(i).functiontype,
                            functions.at(i).active);
            groupFuncs->setId(buttF[buttons], i);
            buttF[buttons]->setCheckable(functions.at(i).buttontype);
            QString FuncToolTip = QString("F%1 %2").arg(i)
                .arg(QCoreApplication::translate("LocoDialog",
                            ftData[functions.at(i).functiontype].ftName));
            buttF[buttons]->setToolTip(FuncToolTip);
            buttons++;
            if (maxfbuttoncount == buttons)
                break;
        }
    }

    // hide remaining unmapped buttons
    for (int i = buttons; i < maxfbuttoncount ; i++) {
        buttF[i]->hide();
    }

    functionLayout->addStretch();

    QWidget* functionsArea = new QWidget();
    functionsArea->setLayout(functionLayout);

    QScrollArea* sv = new QScrollArea();
    Q_CHECK_PTR(sv);
    sv->setWidgetResizable(true);
    sv->setWidget(functionsArea);
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hspeedLayout->addWidget(sv);

    connect(groupFuncs, PRESSED, SLOT(slotUpdateFuncsPressed(int)));
    connect(groupFuncs, RELEASED, SLOT(slotUpdateFuncsReleased(int)));

    /* II middle section with three buttons*/
    //first horizontal layout for button group and single button
    QHBoxLayout* hbreakButtonLayout = new QHBoxLayout();
    hbreakButtonLayout->setObjectName("hbreakButtonLayout");
    hbreakButtonLayout->setSpacing(2);
    vspeedLayout->addLayout(hbreakButtonLayout);

    // second vertical layout for two buttons
    QVBoxLayout* vbreakButtonLayout = new QVBoxLayout();
    vbreakButtonLayout->setObjectName("vbreakButtonLayout");
    vbreakButtonLayout->setSpacing(0);
    hbreakButtonLayout->addLayout(vbreakButtonLayout);

    // break button
    buttBreak = new QPushButton();
    buttBreak->setIcon(QPixmap(brake));
    vbreakButtonLayout->addWidget(buttBreak);
    m_group->addButton(buttBreak, 0);

    buttBreak->setFocusPolicy(Qt::NoFocus);
    buttBreak->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    buttBreak->setToolTip(tr("Brake to halt"));

    // direction button
    buttDirection = new QPushButton();
    buttDirection->setIcon(directionPixmap);
    buttDirection->setFocusPolicy(Qt::NoFocus);
    buttDirection->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    vbreakButtonLayout->addWidget(buttDirection);
    buttDirection->setToolTip(tr("Change direction of travel"));
    connect(buttDirection, SIGNAL(clicked()),
            SLOT(slotReverseDirection()));

    // emergengy break button
    QPixmap pixEmergency = QPixmap(emergency);
    buttEmergency = new QPushButton();
    buttEmergency->setIcon(pixEmergency);
    buttEmergency->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    buttEmergency->setCheckable(true);
    buttEmergency->setFocusPolicy(Qt::NoFocus);
    buttEmergency->setToolTip(tr("Emergency break for this locomotive"));
    hbreakButtonLayout->addWidget(buttEmergency);
    connect(buttEmergency, SIGNAL(toggled(bool)),
            SLOT(slotEmergencyHalt(bool)));

    if (reverseDirection) {
        if (0 == drivemode) /*foreward by reversion */
            directionPixmap = QPixmap(fdirection);
        else
            directionPixmap = QPixmap(bdirection);
    }
    else {
        if (1 == drivemode) /*foreward without reversion */
            directionPixmap = QPixmap(fdirection);
        else
            directionPixmap = QPixmap(bdirection);
    }

    //speed button showing the current speed
    buttSpeed = new QPushButton();
    buttSpeed->setFont(QFont(CFONT, 12, QFont::Bold));
    vspeedLayout->addWidget(buttSpeed);
    buttSpeed->setFocusPolicy(Qt::NoFocus);
    //  updateSpeedButtonText();
    connect(buttSpeed, SIGNAL(clicked()), SLOT(slotToggleSpeedDisplay()));

    /* III bottom section*/
    //frame with locomotive image and name button
    QFrame* frameLoco = new QFrame();
    frameLoco->setMaximumHeight(120);
    frameLoco->setFrameStyle(QFrame::Box | QFrame::Sunken);
    baseLayout->addWidget(frameLoco);

    //layout for loco data
    QVBoxLayout* locoLayout = new QVBoxLayout();
    locoLayout->setObjectName("locoLayout");
    locoLayout->setSpacing(4);
    locoLayout->setContentsMargins(4, 4, 4, 4);
    frameLoco->setLayout(locoLayout);

    QHBoxLayout* lockLayout = new QHBoxLayout();
    lockLayout->setObjectName("lockLayout");
    lockLabel = new QLabel();
    lockLabel->setPixmap(QPixmap(lock_xpm));
    lockLabel->hide();
    lockLayout->addWidget(lockLabel);

    // label for address value
    labelAddress = new QLabel(QString::number(address));
    labelAddress->setFont(QFont(CFONT, 18, QFont::Bold));
    labelAddress->setAlignment(Qt::AlignCenter);
    labelAddress->setFrameStyle(QFrame::NoFrame);
    labelAddress->setToolTip(tr("Decoder address"));
    //locoLayout->addWidget(labelAddress);
    lockLayout->addWidget(labelAddress);
    locoLayout->addLayout(lockLayout);

    // label for loco image
    if (!locoPicture.isEmpty()) {
        bool found = locoPixmap.load(QString(LOCO_PIX_DIR) + locoPicture);
        if (!found)
            emit statusMessage(tr("Error: Pixmap \"%1\" not found.")
                    .arg(locoPicture));
    }
    // Filename im Klartext in loco.dat ohne .xpm !!
    labelPicture = new QLabel();
    labelPicture->setFixedHeight(40);
    labelPicture->setAlignment(Qt::AlignCenter);
    labelPicture->setPixmap(locoPixmap);
    locoLayout->addWidget(labelPicture);

    // button for loco name
    buttName = new QPushButton();
    updateLocoName();

    buttName->setFont(QFont(CFONT, 11, QFont::Bold));
    buttName->setFixedHeight(30);
    locoLayout->addWidget(buttName);
    buttName->setFocusPolicy(Qt::NoFocus);
    connect(buttName, SIGNAL(clicked()), SLOT(slotToggleNameAlias()));
}


void LocoControl::setFuButton(QAbstractButton* fbtn, int functiontype, bool active)
{
    fbtn->setIcon(::getFunctionPixmap(functiontype, active));
    fbtn->setStyleSheet(active ? "background-color: yellow; border: 1px solid #8f8f91"
                               : "background-color: lightgray; border: 1px solid #8f8f91");
}


void LocoControl::updateLocoName()
{
    if (showloconame == NAME) {
        buttName->setText(locoName);
        buttName->setToolTip(tr("Name (loco name)"));
    }
    // (showloconame == ALIAS)
    else {
        buttName->setText(QString::number(trainnumber));
        buttName->setToolTip(tr("CRCF train number"));
    }
}


void LocoControl::slotToggleNameAlias()
{
    showloconame = (NAME == showloconame) ? ALIAS : NAME;
    updateLocoName();
}


void LocoControl::slotToggleSpeedDisplay()
{
    iLocoShowSpeed = !iLocoShowSpeed;
    updateSpeedButtonText();
}


void LocoControl::slotSetAutoSpeed(int which_AutoSpeed)
{
    if (haltTimer->isActive())
        haltTimer->stop();

    if ((iLocoAccelTime == 0) && (iLocoBreakTime == 0)) {
        switch (which_AutoSpeed) {
            case SPEED_HALT:
                speedSlider->setValue(0);
                break;
            case SPEED_MIN:
                speedSlider->setValue(minSpeedStep);
                break;
            case SPEED_MID:
                speedSlider->setValue(avgSpeedStep);
                break;
            case SPEED_MAX:
                speedSlider->setValue(maxSpeedStep);
                break;
        }
    }
    else {
        switch (which_AutoSpeed) {
            case SPEED_HALT:
                setAutoSpeedStep(0);
                break;
            case SPEED_MIN:
                setAutoSpeedStep(minSpeedStep);
                break;
            case SPEED_MID:
                setAutoSpeedStep(avgSpeedStep);
                break;
            case SPEED_MAX:
                setAutoSpeedStep(maxSpeedStep);
                break;
        }
    }
}

/* translate protocol names, return values:
   -1: protocol too short
   -2: unknown protocol main version
   -3: unknown protocol subversion */
int LocoControl::getSrcpGlProtocol(const QString& prot,
        GlMessage::Protocol& result_main, unsigned int& result_sub)
{
    if (prot.size() < 2)
        return -1;

    if (prot.at(0) =='M') {
        result_main = GlMessage::proMM;
        if (prot.at(1) =='1')
            result_sub = 1;
        else
            result_sub = 2;
    }
    else if (prot.at(0) =='N') {
        result_main = GlMessage::proDCC;
        if (prot.at(1) =='3' || prot.at(1) =='4')
            result_sub = 2;
        else
            result_sub = 1;
    }
    else if (prot.at(0) =='P') {
        result_main = GlMessage::proServer;
        result_sub = 1;
    }
    else if (prot.at(0) =='S') {
        result_main = GlMessage::proSlx;
        result_sub = 1;
    }
    else if (prot.at(0) =='X') {
        result_main = GlMessage::proMfx;
        result_sub = 1;
    }
    else
        return -2;

    return 0;
}


void LocoControl::slotEmergencyHalt(bool dohalt)
{
    if (dohalt) {
        delayTimer->stop();
        if (haltTimer->isActive())
            haltTimer->stop();

        // call of setValue is enough to set speed to zero
        //FIXME: drivemode = 2;
        speedSlider->setValue(0);

        // disable drive parameter changes
        speedSlider->setEnabled(false);
        m1->setEnabled(false);
        m2->setEnabled(false);
        m3->setEnabled(false);
        buttBreak->setEnabled(false);
        buttDirection->setEnabled(false);
        srcpstate |= isEmergency;

        GlMessage gm = GlMessage(SrcpMessage::mtGlSet, srcpbus, address,
                functions.size(), 2, currentSpeedStep, decoderSpeedSteps,
                glmfmap);
        emit sendSrcpMessage(gm);
    }
    else {
        speedSlider->setEnabled(true);
        m1->setEnabled(true);
        m2->setEnabled(true);
        m3->setEnabled(true);
        buttBreak->setEnabled(true);
        buttDirection->setEnabled(true);
        buttEmergency->setChecked(false);
        srcpstate &= ~isEmergency;
    }
}


void LocoControl::functionButtonTouched(int fid)
{
    unsigned int bit = 1 << fid;
    if (functions.at(fid).active)
        glmfmap |= bit;
    else
        glmfmap &= ~bit;

    if (functions.at(fid).functiontype > 0) {
        QAbstractButton* fbtn = groupFuncs->button(fid);
        if (fbtn != NULL) {
            fbtn->setChecked(true);
            setFuButton(fbtn, functions.at(fid).functiontype, functions.at(fid).active);
        }
        else
            qWarning("NULL button touched");
    }
    sendLocoState();
}


void LocoControl::slotUpdateFuncsPressed(int fid)
{
    functions[fid].active = !functions.at(fid).active;
    functionButtonTouched(fid);
}


void LocoControl::slotUpdateFuncsReleased(int fid)
{
    if (functions.at(fid).buttontype == PUSHBUTTON) {
    	functions[fid].active = !functions.at(fid).active;
    	functionButtonTouched(fid);
    }
}


void LocoControl::changeFunctionState(int fid)
{
    functions[fid].active = !functions.at(fid).active;

    unsigned int bit = 1 << fid;
    if (functions.at(fid).active)
        glmfmap |= bit;
    else
        glmfmap &= ~bit;

    QAbstractButton* fbtn = groupFuncs->button(fid);
    if (fbtn != NULL) {
        setFuButton(fbtn, functions.at(fid).functiontype, functions.at(fid).active);
    }
}


/*
   001010010 map
   010010001 glmap
   ---------------
   011000011 xor
*/
void LocoControl::updateFunctionStates(unsigned int map)
{
    // check which functions have changed
    unsigned int xormap = glmfmap ^ map;

    for (int i = 0; i < functions.size(); ++i) {
        unsigned int bit = 1 << i;
        if (xormap & bit) {
            changeFunctionState(i);
        }

    }
}


void LocoControl::slotUpdateSpeed(int iNewSpeed)
{
    if (haltTimer->isActive())
        haltTimer->stop();

    currentSpeedStep = iNewSpeed;
    // disable buttDirection if speed != 0
    buttDirection->setEnabled(currentSpeedStep == 0);
    //   buttEmergency->setEnabled( currentSpeedStep != 0 );

    updateSpeedButtonText();
    sendLocoState();
}


void LocoControl::slotReverseDirection()
{
    drivemode = (0 == drivemode) ? 1 : 0;
    updateDirection();
}


void LocoControl::updateDirectionAspect()
{
    // FIXME: show emergeny halt => 2
    if (reverseDirection) {
        if (0 == drivemode) /*foreward by inverting*/
            directionPixmap = QPixmap(fdirection);
        else
            directionPixmap = QPixmap(bdirection);
    }
    else {
        if (1 == drivemode) /*foreward without inverting*/
            directionPixmap = QPixmap(fdirection);
        else
            directionPixmap = QPixmap(bdirection);
    }

    buttDirection->setIcon(directionPixmap);
}


void LocoControl::updateDirection()
{
    updateDirectionAspect();
    directionChanged = false;
    sendLocoState();
}


/* send SRCP command string */
void LocoControl::sendLocoState()
{
    unsigned int prot_sub = 0;
    GlMessage::Protocol prot_main = GlMessage::proDCC;

    int result = getSrcpGlProtocol(locoProtocol, prot_main, prot_sub);

    if (result < 0) {
        qWarning("Protocol conversion error: %d", result);
        return;
    }

    GlMessage gm = GlMessage(SrcpMessage::mtGlSet, srcpbus, address,
            functions.size(), drivemode, currentSpeedStep, decoderSpeedSteps,
            glmfmap);
    emit sendSrcpMessage(gm);
}



void LocoControl::updateSpeedButtonText()
{
    QString speedText;

    if (iLocoShowSpeed == STEPS) {
        speedText = QString("%1").arg(currentSpeedStep);
        buttSpeed->setToolTip(tr("Decoder speedstep"));
    }
    else if (iLocoShowSpeed == REAL) {
        speedText = QString("%1 %2").arg((int)(currentSpeedStep * fLocoSpeedFactor))
                                    .arg(locoSpeedUnit.toLatin1().data());
        buttSpeed->setToolTip(tr("Real speed"));
    }
    buttSpeed->setText(speedText);
}


void LocoControl::runPropertiesDialog()
{
    // stop current speed change action
    delayTimer->stop();
    if (haltTimer->isActive())
        haltTimer->stop();

    // stop locomotive
    speedSlider->setValue(0);
    slotUpdateSpeed(0);

    LocoDialog *dlg = new LocoDialog(this);

    // set all data to dialog parameters
    dlg->setLocoName(locoName);
    dlg->setTrainNumber(trainnumber);
    dlg->setLocoPixmapName(locoPicture);
    dlg->setReverseDirection(reverseDirection);
    dlg->setDecoder(locoDecoder, locoProtocol);
    dlg->setAddress(address);
    dlg->setMfxUid(mfxUid);
    dlg->setAttenuationTimes(iLocoAccelTime, iLocoBreakTime);
    dlg->setLocoSpeeds(maxSpeedStep, avgSpeedStep, minSpeedStep,
            maximumSpeed);
    dlg->setSpeedUnitKmh(locoSpeedUnit == "km/h");
    dlg->setFunctions(functions);
    dlg->setKeepLimit(keeplimit);
    dlg->setFollowLimit(followlimit);

    // redraw controller if settings changed
    if (dlg->exec() == QDialog::Accepted) {

        // read all data from dialog parameter values
        locoName = dlg->getLocoName();
        trainnumber = dlg->getTrainNumber();
        locoPicture = dlg->getLocoPixmapName();
        reverseDirection = dlg->getReverseDirection();
        dlg->getDecoder(locoDecoder, locoProtocol);
        address = dlg->getAddress();
        dlg->getAttenuationTimes(iLocoAccelTime, iLocoBreakTime);
        dlg->getLocoSpeeds(maxSpeedStep, avgSpeedStep, minSpeedStep,
                maximumSpeed);
        if (dlg->getSpeedUnitKmh())
            locoSpeedUnit = "km/h";
        else
            locoSpeedUnit = "mph";
        dlg->getFunctions(functions);
        keeplimit = dlg->getKeepLimit();
        followlimit = dlg->getFollowLimit();

        modified = true;

        updateLocoName();

        if (!locoPicture.isEmpty()) {
            bool found = locoPixmap.load(QString(LOCO_PIX_DIR) + locoPicture);
            if (!found)
                qWarning() << "Pixmap not found:" << locoPicture;
            labelPicture->setPixmap(locoPixmap);
        }
        labelAddress->setText(QString::number(address));

        //FIXME: void mapFunctionButtons()
        int buttons = 0;
        for (int i = 0; i < functions.size(); ++i) {
            if (buttons < maxfbuttoncount && (functions.at(i).functiontype > 0)) {
                setFuButton(buttF[buttons], functions.at(i).functiontype,
                                        functions.at(i).active);
                groupFuncs->setId(buttF[buttons], i);
                buttF[buttons]->setCheckable(functions.at(i).buttontype);
                QString FuncToolTip = QString("F%1 %2").arg(i)
                    .arg(QCoreApplication::translate("LocoDialog",
                                ftData[functions.at(i).functiontype].ftName));
                buttF[buttons]->setToolTip(FuncToolTip);
                buttF[buttons]->show();
                buttons++;
                if (maxfbuttoncount == buttons)
                    break;
            }

            //update function map for GlMessages
            unsigned int bit = 1 << i;
            if (functions.at(i).active)
                glmfmap |= bit;
            else
                glmfmap &= ~bit;
        }
        // hide remaining unmapped buttons
        for (int i = buttons; i < maxfbuttoncount ; i++) {
            buttF[i]->hide();
        }

        speedSlider->setRange(0, maxSpeedStep);
        speedSlider->setTickInterval(maxSpeedStep / 10);
        speedSlider->setValue(0);

        if (maxSpeedStep >= 28)
            speedSlider->setPageStep(maxSpeedStep / 14);
        else
            speedSlider->setPageStep(2);

        decoderSpeedSteps = getDecoderSpeedSteps(locoProtocol);

        QAbstractButton* m1 = m_group->button(1);
        QAbstractButton* m2 = m_group->button(2);
        QAbstractButton* m3 = m_group->button(3);
        float fFaktor = 150.0 / (float) maxSpeedStep;

        m1->setMinimumHeight(25 + qRound(fFaktor * minSpeedStep));
        m2->setMinimumHeight(25 + qRound(fFaktor * (avgSpeedStep - minSpeedStep)));
        m3->setMinimumHeight(25 + qRound(fFaktor * (maxSpeedStep - avgSpeedStep)));

        fLocoSpeedFactor = (float) maximumSpeed / maxSpeedStep;
        updateSpeedButtonText();
        updateDirectionAspect();
        sendLocoState();
    }
    delete dlg;
}


void LocoControl::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        contextmenu->exec(QCursor::pos());
        event->accept();
    }
}


void LocoControl::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    keypressed = true;

    switch (event->key()) {
        case Qt::Key_Space:
            if (buttDirection->isEnabled())
                slotReverseDirection();
            event->accept();
            break;
        case Qt::Key_X:
            if ((event->modifiers() & Qt::ControlModifier) ==
                    Qt::ControlModifier) {
                //if (event->state() == Qt::ControlButton) {
                buttEmergency->animateClick();
                //slotEmergencyHalt();
                event->accept();
            }
            else
                event->ignore();
            break;
        case Qt::Key_B:
            if ((event->modifiers() & Qt::ControlModifier) ==
                    Qt::ControlModifier) {
                //if (event->state() == Qt::ControlButton) {
                buttBreak->animateClick();
                event->accept();
            }
            else
                event->ignore();
            break;
        case Qt::Key_degree:	case Qt::Key_AsciiCircum:
            handleFuncKeyPress(0);
            event->accept();
            break;
        case Qt::Key_F1:
            handleFuncKeyPress(1);
            event->accept();
            break;
        case Qt::Key_F2:
            handleFuncKeyPress(2);
            event->accept();
            break;
        case Qt::Key_F3:
            handleFuncKeyPress(3);
            event->accept();
            break;
        case Qt::Key_F4:
            handleFuncKeyPress(4);
            event->accept();
            break;
        case Qt::Key_F5:
            handleFuncKeyPress(5);
            event->accept();
            break;
        case Qt::Key_F6:
            handleFuncKeyPress(6);
            event->accept();
            break;
        case Qt::Key_F7:
            handleFuncKeyPress(7);
            event->accept();
            break;
        case Qt::Key_F8:
            handleFuncKeyPress(8);
            event->accept();
            break;
        case Qt::Key_F9:
            handleFuncKeyPress(9);
            event->accept();
            break;
        case Qt::Key_F10:
            handleFuncKeyPress(10);
            event->accept();
            break;
        case Qt::Key_F11:
            handleFuncKeyPress(11);
            event->accept();
            break;
        case Qt::Key_F12:
            handleFuncKeyPress(12);
            event->accept();
            break;
        default:
            event->ignore();
            break;
    }
}


void LocoControl::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    keypressed = false;

    switch (event->key()) {
        case Qt::Key_degree:	case Qt::Key_AsciiCircum:
            handleFuncKeyRelease(0);
            event->accept();
            break;
        case Qt::Key_F1:
            handleFuncKeyRelease(1);
            event->accept();
            break;
        case Qt::Key_F2:
            handleFuncKeyRelease(2);
            event->accept();
            break;
        case Qt::Key_F3:
            handleFuncKeyRelease(3);
            event->accept();
            break;
        case Qt::Key_F4:
            handleFuncKeyRelease(4);
            event->accept();
            break;
        case Qt::Key_F5:
            handleFuncKeyRelease(5);
            event->accept();
            break;
        case Qt::Key_F6:
            handleFuncKeyRelease(6);
            event->accept();
            break;
        case Qt::Key_F7:
            handleFuncKeyRelease(7);
            event->accept();
            break;
        case Qt::Key_F8:
            handleFuncKeyRelease(8);
            event->accept();
            break;
        case Qt::Key_F9:
            handleFuncKeyRelease(9);
            event->accept();
            break;
        case Qt::Key_F10:
            handleFuncKeyRelease(10);
            event->accept();
            break;
        case Qt::Key_F11:
            handleFuncKeyRelease(11);
            event->accept();
            break;
        case Qt::Key_F12:
            handleFuncKeyRelease(12);
            event->accept();
            break;
        default:
            event->ignore();
            break;
    }
}


int LocoControl::getDecoderSpeedSteps(const QString& prot)
{
    // default value for protocols M1, M2, M4, NB
    int returnvalue = 14;

    if (prot == "M3")
        returnvalue = 28;
    else if (prot == "M5")
        returnvalue = 27;
    else if (prot == "N1")
        returnvalue = 28;
    else if (prot == "N3")
        returnvalue = 28;
    else if (prot == "N2")
        returnvalue = 128;
    else if (prot == "N4")
        returnvalue = 128;
    else if (prot == "X1")
        returnvalue = 128;

    return returnvalue;
}


int LocoControl::getDecoderFunctionCount(const QString& prot)
{
    // default value for protocols M1, M2, M4, NB
    int returnvalue = 5;

    if (prot == "M1")
        returnvalue = 1;
    else if (prot == "NB")
        returnvalue = 0;

    return returnvalue;
}


void LocoControl::slotSpeedTick()
{
    // no manual- or infoport-manipulation
    if (currentSpeedStep == previousSpeedStep) {
        if (targetSpeedStep > currentSpeedStep)
            speedSlider->setSliderPosition(
                    speedSlider->sliderPosition() +
                    speedSlider->singleStep());
        else if (targetSpeedStep < currentSpeedStep)
            speedSlider->setSliderPosition(
                    speedSlider->sliderPosition() -
                    speedSlider->singleStep());
        else // (targetSpeedStep == currentSpeedStep)
            delayTimer->stop();
        previousSpeedStep = currentSpeedStep;
    }
    else
        delayTimer->stop();
}

void LocoControl::slotOperTick()
{
	if (currentSpeedStep > 2) {
		operationTime += OPER_INTERVAL;
		lastUsed = today;
		modified = true;
	}
}

void LocoControl::slotDelete()
{
    if (QMessageBox::Yes == QMessageBox::warning(this,
                tr("Delete controller"),
                tr("You are going to delete locomotive controller #%1.\n"
                    "Are you sure?").arg(id),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
        emit closed(id);
    }
}


void LocoControl::slotBindRequested()
{
/*
 * SET/VERIFY <bus> SM <decaddress> <smtype> <smaddress> <smvalue>
 *    SET     <bus> SM <decaddress> BIND <uid>
 */

	SmMessage sm = SmMessage(SrcpMessage::mtSmSet, srcpbus, address,
            					SmMessage::smtBind, mfxUid);
    emit sendSrcpMessage(sm);
}


bool LocoControl::hasId(int cid)
{
    return (cid == id);
}


int LocoControl::getId()
{
    return id;
}


// get text, daata and pixmap for list of controllers
QString LocoControl::getListText() const
{
    QString text = QString("%1 (%2)").arg(locoName).arg(trainnumber);
    return text;
}

QString LocoControl::getLastUsed()
{
	return lastUsed;
}

QString LocoControl::getOperationTime()
{
	int h = operationTime / 3600;
	int m = (operationTime % 3600) / 60;
    QString text = QString("%1h:%2m").arg(h, 2).arg(m, 2);
    return text;
}

QPixmap	LocoControl::getLocoPixmap()
{
	return locoPixmap;
}

void LocoControl::handleFuncKeyPress(int fkey)
{
    if (functions.size() <= fkey)
        return;

    if (functions.at(fkey).buttontype == PUSHBUTTON) {
        functions[fkey].active = true;
        functionButtonTouched(fkey);
    }
    else {
        if (functions.at(fkey).functiontype > 0) {
            QAbstractButton* fbtn = groupFuncs->button(fkey);
            if (fbtn != NULL)
                fbtn->click();
        }
        else {
            functions[fkey].active = !functions[fkey].active;
            functionButtonTouched(fkey);
        }
    }
}


void LocoControl::handleFuncKeyRelease(int fkey)
{
    if (functions.size() <= fkey)
        return;

    if (functions.at(fkey).buttontype == PUSHBUTTON) {
        functions[fkey].active = false;
        functionButtonTouched(fkey);
    }
}


void LocoControl::updateDriveAspect()
{
    // update speedSlider
    if (currentSpeedStep != speedSlider->value()) {
        disconnect(speedSlider, SIGNAL(valueChanged(int)), this,
                SLOT(slotUpdateSpeed(int)));
        speedSlider->setValue(currentSpeedStep);
        connect(speedSlider, SIGNAL(valueChanged(int)),
                SLOT(slotUpdateSpeed(int)));
        updateSpeedButtonText();
    }

    // update direction- and emergency-button
    if ((currentSpeedStep == 0) != buttDirection->isEnabled()) {
        // disable buttDirection if speed != 0
        buttDirection->setEnabled(currentSpeedStep == 0);
    }
}

void LocoControl::updateDisplay()
{
    updateDriveAspect();
    updateDirectionAspect();

    // update functions, respect id mapping functions <-> buttons
    for (int i = 0; i < functions.size(); ++i) {
        if (functions.at(i).buttontype == SWITCH) {
            QAbstractButton* fbtn = groupFuncs->button(i);
            if (fbtn == NULL)
                continue;

            if (functions.at(i).active != fbtn->isDown()) {
                setFuButton(fbtn, functions.at(i).functiontype, functions.at(i).active);
            }
        }
    }
}

//handle incomming GL INFO message for this bus
void LocoControl::handleGlInfoMessage(const GlMessage* glm)
{
    switch (glm->getCode()) {
        case SrcpMessage::code100: //SET
            if (glm->hasAddress(address)) {
                if (!glm->matchFunctionMap(glmfmap)) {
                    updateFunctionStates(glm->getFunctionMap());
                }
                // only update speed and direction if not in timer mode
                // and slider is not shifted
                if (!keypressed && !delayTimer->isActive() &&
                        !speedSlider->isSliderDown()) {
                    unsigned int dm;
                    unsigned int cv;
                    unsigned int maxv;
                    glm->getDriveSet(dm, cv, maxv);
                    if (maxv == (unsigned int)decoderSpeedSteps) {
                        if ((unsigned int) currentSpeedStep != cv) {
                            currentSpeedStep = cv;
                            updateDriveAspect();
                        }
                        if ((drivemode != dm) && (dm < 2)) {
                            drivemode = dm;
                            updateDirectionAspect();
                        }
                    }
                    else
                        qWarning("INFO: %d not matching decoder %d steps: %d",
                                address, decoderSpeedSteps, maxv);
                }
                else {
                    //what to do if timer is active?
                }

            }
            break;
        case SrcpMessage::code101: //INIT
            // check if protocol and function count are equal to own
            // configuration; if not ignore code100 messages
            if (glm->hasAddress(address)) {
            }
            break;
        case SrcpMessage::code102: //TERM
            // deactivate controller?
            if (glm->hasAddress(address)) {
            }
            break;
        default:
            break;
    }
}


/*request step by step missing CRCF data of current route,
 * response goes to handleCrcfRouteMessage() */
void LocoControl::completeRouteData(unsigned int route)
{
    Route* rt = NULL;

    for (int i = 0; i < routes.size(); i++) {
        if (routes.at(i).id == route)
            rt = &routes[i];
    }
    if (NULL == rt)
        return;

    completeRouteData(rt);
}

/*request step by step missing CRCF data of current route,
 * response goes to handleCrcfRouteMessage() */
void LocoControl::completeRouteData(Route* rt)
{
    CrcfMessage cm = CrcfMessage(CrcfMessage::acRoute, rt->id,
            CrcfMessage::meGet, CrcfMessage::atNone);

    // 1. Route length
    // 2. Route speedlimit
    // 3. Route exit signal aspect
    // 4. Route entry signal aspect (only for next route)
    // Train id is not requested as it triggers this request
    switch (rt->request) {
        case requestNone:
            rt->request = requestName;
            cm.setAttribute(CrcfMessage::atLength);
            // FIXME: How to handle time out?
            break;
        case requestName:
            rt->request = requestLength;
            cm.setAttribute(CrcfMessage::atName);
            break;
        case requestLength:
            rt->request = requestSpeedlimit;
            cm.setAttribute(CrcfMessage::atSpeedLimit);
            break;
        case requestSpeedlimit:
            rt->request = requestEntrysignalaspect;
            cm.setAttribute(CrcfMessage::atEntrySignalAspect);
            break;
        case requestEntrysignalaspect:
            rt->request = requestExitsignalaspect;
            cm.setAttribute(CrcfMessage::atExitSignalAspect);
            break;
        case requestExitsignalaspect:
            rt->request = requestNone;
            break;
    }

    if (rt->request != requestNone) {
        GmMessage gmm = GmMessage(SrcpMessage::mtGmSet, gmreplyid,
                infosessionid, "CRCF", cm.getMessage());
        emit sendSrcpMessage(gmm);
    }
}

/*Handle train direction change messages*/
void LocoControl::handleCrcfTrainDirectionMessage(const CrcfMessage* cm)
{
    /*for now respond only to SET methods*/
    if (CrcfMessage::meSet != cm->getMethod())
        return;

    switch (cm->getAttValue()) {
        case tdirBackward:
            if (buttDirection->isEnabled()) {
                drivemode = 0;
                updateDirection();
            }
            else
                emit statusMessage(
                        tr("Direction change not possible"));
            break;
        case tdirForward:
            if (buttDirection->isEnabled()) {
                drivemode = 1;
                updateDirection();
            }
            else
                emit statusMessage(
                        tr("Direction change not possible"));
            break;
        case tdirEmergency:
            buttEmergency->toggle();
            break;
        case tdirToggleDir:
            if (buttDirection->isEnabled())
                buttDirection->click();
            else
                emit statusMessage(
                        tr("Direction change not possible"));
            break;
        default:
            break;
    }
}

// handle incomming CRCF TRAIN messages
// CRCF TRAIN <id> SET/INFO <attribute> <value>
void LocoControl::handleCrcfTrainMessage(const CrcfMessage* cm)
{
    if (trainnumber != cm->getActorId())
        return;

    switch (cm->getAttribute()) {
        case CrcfMessage::atDirection:
            handleCrcfTrainDirectionMessage(cm);
            break;
        case CrcfMessage::atNextRoute:
            if (CrcfMessage::meSet == cm->getMethod() ||
                    CrcfMessage::meInfo == cm->getMethod()) {
                setNextRoute(cm->getAttValue());
            }
            break;
        default:
            break;
    }
}

/*Get pointer to route by route id*/
Route* LocoControl::getRouteById(unsigned int id)
{
    for (int i = 0; i < routes.size(); i++) {
        if (routes.at(i).id == id)
            return &routes[i];
    }
    return NULL;
}


// CRCF ROUTE <routeid> INFO STATE 0/1
void LocoControl::handleCrcfRouteStateMessage(const CrcfMessage* cm)
{
    if (CrcfMessage::meInfo != cm->getMethod())
        return;

    Route* rt = getRouteById(cm->getActorId());
    if (NULL == rt)
        return;

    switch (cm->getAttValue()) {
        case 0:
            setRouteEntrySignalAspect(*rt, saHp0);
            {
            rt->request = requestExitsignalaspect;
            CrcfMessage cm = CrcfMessage(CrcfMessage::acRoute, rt->id,
                    CrcfMessage::meGet, CrcfMessage::atExitSignalAspect);
            GmMessage gmm = GmMessage(SrcpMessage::mtGmSet, gmreplyid,
                    infosessionid, "CRCF", cm.getMessage());
            emit sendSrcpMessage(gmm);
            }
            break;
        case 1:
            {
            rt->request = requestEntrysignalaspect;
            CrcfMessage cm = CrcfMessage(CrcfMessage::acRoute, rt->id,
                    CrcfMessage::meGet, CrcfMessage::atEntrySignalAspect);
            GmMessage gmm = GmMessage(SrcpMessage::mtGmSet, gmreplyid,
                    infosessionid, "CRCF", cm.getMessage());
            emit sendSrcpMessage(gmm);
            }
            break;
        default:
            break;
    }
}


void LocoControl::checkForNextRoute(Route* rt)
{
    // request next route if no next route is available and
    // exitsignal shows go
    if (nextroute == 0 && rt->exitsignalaspect != saHp0) {
        CrcfMessage cm = CrcfMessage(CrcfMessage::acTrain, trainnumber,
                CrcfMessage::meGet, CrcfMessage::atNextRoute);
        GmMessage gmm = GmMessage(SrcpMessage::mtGmSet, gmreplyid,
                infosessionid, "CRCF", cm.getMessage());
        emit sendSrcpMessage(gmm);
    }
}


void LocoControl::handleCrcfRouteTrainMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            // clear currently used route
            // CRCF ROUTE <routeid> INFO TRAIN 0
            if (0 == cm->getAttValue() &&
                    currentroute == cm->getActorId())
                setCurrentRoute(0);

            // update currently used route
            // CRCF ROUTE <routeid> INFO TRAIN <my_trainid>
            else if (trainnumber == cm->getAttValue()) {
                // create new route if id not known yet
                setCurrentRoute(cm->getActorId());
                Route* rt = getRouteById(cm->getActorId());
                if (NULL == rt)
                    break;

                checkForNextRoute(rt);
            }
            break;
        default:
            break;
    }
}


void LocoControl::handleCrcfRouteNameMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            {
            // CRCF ROUTE <routeid> SET/INFO NAME <name>
            Route* rt = getRouteById(cm->getActorId());
            if (NULL == rt)
                break;

            setRouteName(*rt, cm->getAttValueStr());
            if (currentroute == cm->getActorId()) {
                if (requestNone == rt->request)
                    handleRouteUpdate();
                else
                    completeRouteData(rt);
            }
            else if (nextroute == cm->getActorId())
                if (requestNone != rt->request)
                    completeRouteData(rt);
            }
            break;
        default:
            break;
    }
}


void LocoControl::handleCrcfRouteLengthMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            {
            // CRCF ROUTE <routeid> SET/INFO LENGTH <length>
            Route* rt = getRouteById(cm->getActorId());
            if (NULL == rt)
                break;
            setRouteLength(*rt, cm->getAttValue());
            if (currentroute == cm->getActorId()) {
                if (requestNone == rt->request)
                    handleRouteUpdate();
                else
                    completeRouteData(rt);
            }
            else if (nextroute == cm->getActorId())
                if (requestNone != rt->request)
                    completeRouteData(rt);
            }
            break;
        default:
            break;
    }
}


void LocoControl::handleCrcfRouteSpeedLimitMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            {
            // CRCF ROUTE <routeid> SET/INFO SPEEDLIMIT <limit>
            Route* rt = getRouteById(cm->getActorId());
            if (NULL == rt)
                break;
            setRouteSpeedLimit(*rt, cm->getAttValue());
            if (currentroute == cm->getActorId()) {
                if (requestNone == rt->request)
                    handleRouteUpdate();
                else
                    completeRouteData(rt);
            }
            else if (nextroute == cm->getActorId())
                if (requestNone != rt->request)
                    completeRouteData(rt);
            }
            break;
        default:
            break;
    }
}


void LocoControl::handleCrcfRouteEntrySignalAspectMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            {
            Route* rt = getRouteById(cm->getActorId());
            if (NULL == rt)
                break;
            setRouteEntrySignalAspect(*rt, cm->getAttValue());
            if (currentroute == cm->getActorId()) {
                if (requestNone == rt->request)
                    handleRouteUpdate();
                else
                    completeRouteData(rt);
            }
            else if (nextroute == cm->getActorId())
                if (requestNone != rt->request)
                    completeRouteData(rt);

            //check if signal > Hp0 and belongs to next route, give a go
            if (nextroute == 0)
                break;

            if (nextroute != cm->getActorId())
                break;

            if (0 == rt->speedlimit) {
                emit statusMessage(
                        tr("Error: Route without speed limit found: %1")
                        .arg(nextroute));
                break;
            }

            if (saHp0 == cm->getAttValue())
                break;

            if (followlimit)
                followSpeedLimit(rt->speedlimit);
            else
                keepSpeedLimit(rt->speedlimit);
            }
            break;
        default:
            break;
    }
}


void LocoControl::handleCrcfRouteExitSignalAspectMessage(const CrcfMessage* cm)
{
    switch (cm->getMethod()) {
        case CrcfMessage::meSet:
        case CrcfMessage::meInfo:
            {
            // CRCF ROUTE <routeid> SET/INFO EXITSIGNALASPECT <aspect>
            Route* rt = getRouteById(cm->getActorId());
            if (NULL == rt)
                break;

            setRouteExitSignalAspect(*rt, cm->getAttValue());
            if (currentroute == cm->getActorId()) {
                // aspect change is last requested route data; first
                // make completion check then act according to
                // current signal aspect
                if (requestNone != rt->request)
                    completeRouteData(rt);

                handleRouteUpdate();
                checkForNextRoute(rt);
            }
            else if (nextroute == cm->getActorId())
                if (requestNone != rt->request)
                    completeRouteData(rt);
            }
            break;
        default:
            break;
    }
}


// handle incomming CRCF ROUTE messages
// CRCF <actor> <actorid> <action> <attribute> <attributevalue>
// CRCF ROUTE 2 SET/INFO TRAIN 2100
// CRCF ROUTE 6 INFO LENGTH 1300
// CRCF ROUTE 6 INFO SPEEDLIMIT 40
void LocoControl::handleCrcfRouteMessage(const CrcfMessage* cm)
{
    switch (cm->getAttribute()) {
        case CrcfMessage::atState:
            handleCrcfRouteStateMessage(cm);
            break;
        case CrcfMessage::atTrain:
            handleCrcfRouteTrainMessage(cm);
            break;
        case CrcfMessage::atName:
            handleCrcfRouteNameMessage(cm);
            break;
        case CrcfMessage::atLength:
            handleCrcfRouteLengthMessage(cm);
            break;
        case CrcfMessage::atSpeedLimit:
            handleCrcfRouteSpeedLimitMessage(cm);
            break;
        case CrcfMessage::atEntrySignalAspect:
            handleCrcfRouteEntrySignalAspectMessage(cm);
            break;
        case CrcfMessage::atExitSignalAspect:
            handleCrcfRouteExitSignalAspectMessage(cm);
            break;
        default:
            break;
    }
}

// handle incomming GM INFO message
// CRCF: <actor> <actor_id> <method> <attribute> [<attribute_value>]
// 1455991042.550 100 INFO 0 GM 0 20 CRCF ROUTE 2 INFO TRAIN 2100
void LocoControl::handleGmInfoMessage(const GmMessage* gmm)
{
    unsigned int sendid = gmm->getSendId();
    unsigned int replyid = gmm->getReplyId();

    //shortcut not to reply to own messages
    if (infosessionid == replyid)
        return;

    // we are not the addressee
    if (sendid != 0 && infosessionid != sendid)
        return;

    // save GM reply id to address potential replies
    gmreplyid = replyid;

    CrcfMessage* cm = gmm->createCrcfMessage();
    if (NULL != cm) {

        switch (cm->getActor()) {
            case CrcfMessage::acRoute:
                handleCrcfRouteMessage(cm);
                break;
            case CrcfMessage::acTrain:
                handleCrcfTrainMessage(cm);
                break;
            default:
                break;
        }
        delete cm;
    }
    else
        emit statusMessage(tr("Unvalid CRCF message received"));
}


void LocoControl::processInfoMessage(const SrcpMessage* sm)
{
    switch (sm->getType()) {
        case SrcpMessage::mtAckHandShake:
            infosessionid = ((AckMessage*)sm)->getSessionId();
            break;
        case SrcpMessage::mtGlInfo:
            if (srcpbus == sm->getBus())
                handleGlInfoMessage(((GlMessage*)sm));
            break;
        case SrcpMessage::mtGmInfo:
            handleGmInfoMessage(((GmMessage*)sm));
            break;
        case SrcpMessage::mtDescriptionInfo:
            if (srcpbus == sm->getBus()) {
                if (!((DescriptionMessage*)sm)->hasGl())
                    emit statusMessage(tr("Error: Bus %1 does not"
                                " support GL devices!").arg(srcpbus));
            }
            break;
        case SrcpMessage::mtPowerInfo:
            // reset emergency break state
            if (((PowerMessage*)sm)->busison(srcpbus) > 0)
            	slotEmergencyHalt(false);
            break;
        default:
            break;
    }
}


void LocoControl::processCommandMessage(const SrcpMessage* sm)
{
    switch (sm->getType()) {

        case SrcpMessage::mtAckHandShake:
            commandsessionid = ((AckMessage*)sm)->getSessionId();
            break;

        case SrcpMessage::mtAckCommand: {
            SrcpMessage::Code code = sm->getCode();
            if (SrcpMessage::code200 == code) {
                if (srcpstate & lockRequested) {
                    srcpstate &= ~lockRequested;
                    srcpstate |= isLocked;
                    lockLabel->show();
                    lockAction->setText(tr("&Unlock"));
                }
                else if (srcpstate & lockTermRequested) {
                    srcpstate &= ~lockTermRequested;
                    srcpstate &= ~isLocked;
                    lockLabel->hide();
                    lockAction->setText(tr("&Lock"));
                }
            }
            else if (SrcpMessage::code414 == code) {
                if (srcpstate & lockRequested) {
                    srcpstate &= ~lockRequested;
                    emit statusMessage(tr("Lock not acquired (error 414)."));
                }
                else if (srcpstate & lockTermRequested) {
                    srcpstate &= ~lockTermRequested;
                }
            }
            }
            break;
        default:
            break;
    }
}


void LocoControl::writeConfig(QTextStream& ts)
{
    ts << "LOCO " << id << ' ' << !isVisible() << endl
       << DF_ADDRESS << ':' << address << endl
       << DF_NAME << ':' << locoName << endl
       << DF_ALIAS << ':' << trainnumber << endl
       << DF_SHOWWHAT << ':'
       << ((showloconame == NAME) ? "name" : "alias") << endl
       << DF_ICON << ':' << locoPicture << endl
       << DF_PROTOCOL << ':' << locoProtocol << endl
       << DF_DECODER << ':' << locoDecoder << endl;

    if (mfxUid) ts << DF_MFXUID << ':' << mfxUid << endl;

    for (int i = 0; i < functions.size(); i++) {
        ts << DF_F_FTYPE << ':' << functions.at(i).functiontype << endl
           << DF_F_STATE << ':' << functions.at(i).active << endl
           << DF_F_BTYPE << ':' << functions.at(i).buttontype << endl;
    }

    ts << DF_DIRECTION << ':' << ((drivemode == 1) ? "fwd" : "rev") << endl
       << DF_REVDIRECTION << ':' << reverseDirection << endl
       << DF_KEEPLIMIT << ':' << keeplimit << endl
       << DF_FOLLOWLIMIT << ':' << followlimit << endl
       << DF_SHOWSPEED << ':' << ((iLocoShowSpeed > 0) ? "real" :
                                      "steps") << endl
       << DF_SPEEDUNIT << ':' << locoSpeedUnit << endl
       << DF_REALMAXSPEED << ':' << maximumSpeed << endl
       << DF_MINSPEED << ':' << minSpeedStep << endl
       << DF_AVGSPEED << ':' << avgSpeedStep << endl
       << DF_MAXSPEED << ':' << maxSpeedStep << endl
       << DF_ACCELTIME << ':' << iLocoAccelTime << endl
       << DF_BREAKTIME << ':' << iLocoBreakTime << endl
       << DF_RVNAME << ':' << rvo.showname << endl
       << DF_RVENTRYASPECT << ':' << rvo.showentryaspect << endl
       << DF_RVEXITASPECT << ':' << rvo.showexitaspect << endl
       << DF_RVLENGTH << ':' << rvo.showlength << endl
       << DF_RVSPEED << ':' << rvo.showspeed << endl
       << DF_RVLIMITROUTES << ':' << rvo.limitroutes << endl
       << DF_RVROUTELIMIT << ':' << rvo.routelimit << endl
	   << DF_OPERTIME << ':' << operationTime << endl
	   << DF_LASTUSED << ':' << lastUsed << endl
       << '-' << endl;

    modified = false;
}

/*
 * read loco data from text stream
 */
void LocoControl::readFileTextFromStream(QTextStream& ts)
{
    QString s, key, value;
    int currentfunction = -1;

    while (!ts.atEnd()) {
        s = ts.readLine();
        if (!s.startsWith("#")) {
            /*break if end of single loco data set*/
            if (s.startsWith("-") || s.startsWith("%"))
                break;

            key = s.section(':', 0, 0);
            value = s.section(':', 1).trimmed();

            if (key.compare(DF_ADDRESS) == 0){
                address = value.trimmed().toUInt();
            }
            else if (key.compare(DF_NAME) == 0){
                locoName = value.trimmed();
            }
            else if (key.compare(DF_ALIAS) == 0){
                trainnumber = value.trimmed().toUInt();
            }
            else if (key.compare(DF_SHOWWHAT) == 0){
                showloconame = (value.trimmed() == "name")  ? NAME : ALIAS;
            }
            else if (key.compare(DF_ICON) == 0){
                locoPicture = value.trimmed();
            }
            else if (key.compare(DF_PROTOCOL) == 0){
                locoProtocol = value.trimmed();
                decoderSpeedSteps = getDecoderSpeedSteps(locoProtocol);
            }
            else if (key.compare(DF_DECODER) == 0){
                locoDecoder = value.trimmed();
            }
            else if (key.compare(DF_MFXUID) == 0){
                mfxUid = value.trimmed().toUInt();
            }
            else if (key.compare(DF_DIRECTION) == 0){
                drivemode = (value.trimmed() == "fwd") ? 1 : 0;
            }
            else if (key.compare(DF_REVDIRECTION) == 0){
                reverseDirection = value.trimmed().toUInt();
            }
            else if (key.compare(DF_KEEPLIMIT) == 0){
                keeplimit = (value.trimmed().toInt() == 1);
            }
            else if (key.compare(DF_FOLLOWLIMIT) == 0){
                followlimit = (value.trimmed().toInt() == 1);
            }
            else if (key.compare(DF_SHOWSPEED) == 0){
                iLocoShowSpeed = (value.trimmed() == "real") ? 1 : -1;
            }
            else if (key.compare(DF_SPEEDUNIT) == 0){
                locoSpeedUnit = value.trimmed();
            }
            else if (key.compare(DF_REALMAXSPEED) == 0){
                maximumSpeed = value.trimmed().toInt();
            }
            else if (key.compare(DF_MINSPEED) == 0){
                minSpeedStep = value.trimmed().toInt();
            }
            else if (key.compare(DF_AVGSPEED) == 0){
                avgSpeedStep = value.trimmed().toInt();
            }
            else if (key.compare(DF_MAXSPEED) == 0){
                maxSpeedStep = value.trimmed().toInt();
            }
            else if (key.compare(DF_ACCELTIME) == 0){
                iLocoAccelTime = value.trimmed().toInt();
            }
            else if (key.compare(DF_BREAKTIME) == 0){
                iLocoBreakTime = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVNAME) == 0){
                rvo.showname = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVENTRYASPECT) == 0){
                rvo.showentryaspect = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVEXITASPECT) == 0){
                rvo.showexitaspect = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVLENGTH) == 0){
                rvo.showlength = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVSPEED) == 0){
                rvo.showspeed = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVLIMITROUTES) == 0){
                rvo.limitroutes = value.trimmed().toInt();
            }
            else if (key.compare(DF_RVROUTELIMIT) == 0){
                rvo.routelimit = value.trimmed().toInt();
            }
            else if (key.compare(DF_OPERTIME) == 0){
	   			operationTime = value.trimmed().toUInt();
            }
            else if (key.compare(DF_LASTUSED) == 0){
	   			lastUsed = value.trimmed();
	   		}

            // new functions
            else if (key.compare(DF_F_FTYPE) == 0){
            	int ft = value.toInt();
                functions << Function(ft, PUSHBUTTON, false);
                currentfunction = functions.size() - 1;
            }
            else if ((key.compare(DF_F_STATE) == 0) && (currentfunction >= 0)){
                functions[currentfunction].active = value.toInt();
                unsigned int bit = 1 << currentfunction;
                if (functions.at(currentfunction).active)
                    glmfmap |= bit;
                else
                    glmfmap &= ~bit;
            }
            else if ((key.compare(DF_F_BTYPE) == 0) && (currentfunction >= 0)){
                functions[currentfunction].buttontype = value.toInt();
            }
        }
    }

    //at least update a calculated value
    fLocoSpeedFactor = (float) maximumSpeed / maxSpeedStep;
}

bool LocoControl::isModified()
{
    return modified;
}

void LocoControl::updateSrcpBus(unsigned int bus)
{
    srcpbus = bus;
}


void LocoControl::updateScale(int sc)
{
    setScale(sc);
}


//send controller init to server
void LocoControl::initialize()
{
    GlMessage::Protocol prot_main = GlMessage::proDCC;
    unsigned int prot_sub = 0;

    int result = getSrcpGlProtocol(locoProtocol, prot_main,
            prot_sub);

    if (result < 0) {
        qWarning("Protocol conversion error: %d", result);
        return;
    }
    GlMessage glm = GlMessage(SrcpMessage::mtGlInit,
            srcpbus, address, prot_main, prot_sub,
            decoderSpeedSteps, functions.size());
    emit sendSrcpMessage(glm);
}


void LocoControl::setInitialized()
{
    srcpstate |= initDone;
    lockAction->setEnabled(true);
}


bool LocoControl::initialized()
{
    return ((srcpstate & initDone) == initDone);
}


bool LocoControl::emergencyHalted()
{
    return ((srcpstate & isEmergency) == isEmergency);
}

//reset after server is disconnected
void LocoControl::reinitialize()
{
    srcpstate &= ~initDone;
    lockAction->setEnabled(false);
}

//Toggle SRCP lock state for this controller; if is locked show lock icon

void LocoControl::toggleControllerLock()
{
    if (srcpstate & isLocked) {
        // terminate lock
        srcpstate |= lockTermRequested;
        LockMessage lm = LockMessage(SrcpMessage::mtLockTerm,
                srcpbus, SrcpMessage::devGl, address);
        emit sendSrcpMessage(lm);
    }
    else {
        // request lock without time limit
        srcpstate |= lockRequested;
        LockMessage lm = LockMessage(SrcpMessage::mtLockSet,
                srcpbus, SrcpMessage::devGl, address, 0);
        emit sendSrcpMessage(lm);
    }
}


/*Show dialog with route list*/
void LocoControl::showRoutes()
{
    if (NULL == routesWindow) {
        routesWindow = new RoutesWindow(nativeParentWidget(), locoName,
                QString::number(trainnumber), rvo);
        routesWindow->setRouteList(routes);
        connect(this, SIGNAL(routeDataChanged(const Route&)),
                routesWindow, SLOT(updateRouteData(const Route&)));
        //FIXME: this typecast may cause a crash on KDE/Qt4 systems
        ((QMainWindow *)nativeParentWidget())->addDockWidget(
        Qt::RightDockWidgetArea, routesWindow);
    }
    routesWindow->show();
    routesWindow->raise();
}

/*Clear collected route data*/
void LocoControl::clearRoutes()
{
    if (NULL != routesWindow) {
        routesWindow->clearRoutes();
    }
    routes.clear();
}

/*run dialog to edit routes view options*/
void LocoControl::editRoutesViewOptions()
{
    RoutesViewOptionsDialog *dlg = new RoutesViewOptionsDialog(this);
    dlg->setRoutesViewOptions(rvo);
    if (dlg->exec() == QDialog::Accepted) {
        dlg->getRoutesViewOptions(rvo);
        if (rvo.limitroutes) {
            while (routes.size() > (int)rvo.routelimit)
                routes.removeFirst();
        }
        if (NULL != routesWindow)
            routesWindow->setRouteViewOptions(rvo);
        modified = true;
    }
    delete dlg;
}

/* Update data for next route*/
void LocoControl::setNextRoute(unsigned int route)
{
    nextroute = route;
    bool found = false;

    // no further action if route == 0 (train left last route)
    if (route == 0)
        return;

    for (int i = 0; i < routes.size(); i++) {
        if (routes.at(i).id == route) {
            routes[i].state = rsActivated;
            emit routeDataChanged(routes[i]);
            found = true;
        }
    }

    // add data for unknown route
    if (!found) {
        Route nr;
        nr.id = route;
        nr.name = tr("(None)");
        nr.length = 0;
        nr.speedlimit = 0;
        nr.entrysignalaspect = saNone;
        nr.exitsignalaspect = saNone;
        nr.state = rsActivated;
        nr.request = requestNone;
        routes << nr;
        if (rvo.limitroutes) {
            while (routes.size() > (int)rvo.routelimit)
                routes.removeFirst();
        }
        emit routeDataChanged(nr);
        completeRouteData(route);
    }
}

/* Update data for currently used route*/
void LocoControl::setCurrentRoute(unsigned int cr)
{
    currentroute = cr;
    if (nextroute == cr)
        nextroute = 0;
    bool found = false;

    for (int i = 0; i < routes.size(); i++) {
        if (routes.at(i).id == cr) {
            routes[i].state = rsUsed;
            emit routeDataChanged(routes[i]);
            found = true;
            handleRouteUpdate();
        }
        else
            if (routes.at(i).state == rsUsed) {
                 routes[i].state = rsUnused;
                 emit routeDataChanged(routes[i]);
            }
    }

    // no further action if route == 0 (train left last route)
    if (cr == 0)
        return;

    // add data for unknown route
    if (!found) {
        Route nr;
        nr.id = cr;
        nr.name = tr("(None)");
        nr.length = 0;
        nr.speedlimit = 0;
        nr.entrysignalaspect = saNone;
        nr.exitsignalaspect = saNone;
        nr.state = rsUsed;
        nr.request = requestNone;
        routes << nr;
        if (rvo.limitroutes) {
            while (routes.size() > (int)rvo.routelimit)
                routes.removeFirst();
        }
        emit routeDataChanged(nr);
        completeRouteData(cr);
    }
}

/* Update name of (a requested?) route*/
void LocoControl::setRouteName(Route& rt, const QString& rname)
{
    rt.name = rname;
    emit routeDataChanged(rt);
}

/* Update length for (a requested?) route*/
void LocoControl::setRouteLength(Route& rt, unsigned int rlength)
{
    rt.length = rlength;
    emit routeDataChanged(rt);
}

/* Update speedlimit for (a requested?) route*/
void LocoControl::setRouteSpeedLimit(Route& rt, unsigned int rsl)
{
    rt.speedlimit = rsl;
    emit routeDataChanged(rt);
}

/* Update entry signal aspect for (a requested?) route*/
void LocoControl::setRouteEntrySignalAspect(Route& rt, unsigned int esa)
{
    rt.entrysignalaspect = esa;
    emit routeDataChanged(rt);
}

/* Update exit signal aspect for (a requested?) route*/
void LocoControl::setRouteExitSignalAspect(Route& rt, unsigned int esa)
{
    rt.exitsignalaspect = esa;
    emit routeDataChanged(rt);
}

/*
 respond to route change (route id/signal aspect)
   1) adjust current speed to limit if limit is available
   2) initiate stop in front of route exit signal exposing Hp0 aspect
   3) go if route exit signal changes to Hp1/2/Sh1 aspect
 */
void LocoControl::handleRouteUpdate()
{
    if (0 == currentroute)
        return;

    Route* cr = NULL;
    for (int i = 0; i < routes.size(); i++) {
        if (routes.at(i).id == currentroute)
            cr = &routes[i];
    }
    if (NULL == cr)
        return;

    if (0 == cr->speedlimit) {
        emit statusMessage(tr("Error: Route without speed limit found: %1")
                .arg(currentroute));
        return;
    }

    if (0 == cr->length) {
        emit statusMessage(tr("Error: Route without length found: %1")
                .arg(currentroute));
        return;
    }

    switch (cr->exitsignalaspect) {
        case saHp0:
            handleHp0(cr->length);
            break;
        case saNone:
            emit statusMessage(
                    tr("Error: Route without exit signal aspect found: %1")
                    .arg(currentroute));
            break;
        default:
            if (followlimit)
                followSpeedLimit(cr->speedlimit);
            else
                keepSpeedLimit(cr->speedlimit);
            break;
    }
}

/* calculate where to halt if exit signal shows Hp0*/
void LocoControl::handleHp0(unsigned int length)
{
    if (0 == currentSpeedStep)
        return;

    float timetosignal = (scaledivisor * 36 * length)/
        (10000.0 * currentSpeedStep * fLocoSpeedFactor);

    float timetobreak = (currentSpeedStep * iLocoBreakTime)/1000.0;
    emit statusMessage(tr("Time to exit signal: %1 s, time for breaking: %2 s")
            .arg(timetosignal, 0 , 'f', 2)
            .arg(timetobreak, 0 , 'f', 2));

    float waittime = timetosignal - timetobreak;
    if (waittime > 0.0)
        // some time to wait util breaking starts (s -> ms)
        //TODO: check where to stop this timer in case of manual speed change
        haltTimer->start(waittime * 1000);
    else {
        // emergency case, do fast halt
        setAutoSpeedStep(0);
        emit statusMessage(tr("Emergency break case: %1 s missing")
                .arg(-waittime, 0 , 'f', 2));
    }
}

/*initiate delayed speed change to new speed target*/
void LocoControl::setAutoSpeedStep(int nss)
{
    targetSpeedStep = nss;
    previousSpeedStep = currentSpeedStep;
    delayTimer->stop();
    if (targetSpeedStep > currentSpeedStep) {
        slotSpeedTick();
        delayTimer->start(iLocoAccelTime);
    }
    if (targetSpeedStep < currentSpeedStep) {
        slotSpeedTick();
        delayTimer->start(iLocoBreakTime);
    }
}

/* decrease speed to limit if necessary (if option is set)*/
void LocoControl::keepSpeedLimit(int sl)
{
    if (haltTimer->isActive()) {
        haltTimer->stop();
        emit statusMessage(tr("Break timing cancelled."));
    }

    if (!keeplimit)
        return;

    if ((currentSpeedStep * fLocoSpeedFactor) > sl)
        setAutoSpeedStep(sl/fLocoSpeedFactor);
}

/* adjust speed to limit if necessary (if option is set)*/
/*TODO: Check current halt condition not to start moving in front of Hp0*/
void LocoControl::followSpeedLimit(int sl)
{
    if (haltTimer->isActive()) {
        haltTimer->stop();
        emit statusMessage(tr("Break timing cancelled."));
    }

    if ((currentSpeedStep * fLocoSpeedFactor) != sl)
        setAutoSpeedStep(sl/fLocoSpeedFactor);
}

/* set scale and scaledivisor value*/
void LocoControl::setScale(int sc)
{
    if (scale == sc)
        return;

    scale = sc;
    switch (sc) {
        case scale2:
            scaledivisor = 22.5;
            break;
        case scale1:
            scaledivisor = 32.0;
            break;
        case scale0:
            scaledivisor = 45.0;
            break;
        case scaleS:
            scaledivisor = 64.0;
            break;
        case scaleH0:
            scaledivisor = 87.0;
            break;
        case scaleTT:
            scaledivisor = 120.0;
            break;
        case scaleN:
            scaledivisor = 160.0;
            break;
        case scaleZ:
            scaledivisor = 220.0;
            break;
        default:
            scaledivisor = 87.0;
            break;
    }
}

/*haltTimer action to halt locomotive on time*/
void LocoControl::executeHalt()
{
    setAutoSpeedStep(0);
}

