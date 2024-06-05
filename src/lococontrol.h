// lococontrol.h - adapted for raidtcl project 2018 - 2021 by Rainer Müller

/***************************************************************************
                               LocoControl.h
                             -----------------
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

#ifndef LOCOCONTROL_H
#define LOCOCONTROL_H

#include <QApplication>
#include <QButtonGroup>
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QScrollArea>

#include "locodialog.h"
#include "routeswindow.h"
#include "routesviewoptionsdialog.h"
#include "srcpmessage.h"


enum speedTypes
{
    STEPS,
    REAL
};

enum SrcpState
{
    stateNone = 0,
    initDone = 1,
    lockRequested = 2,
    lockTermRequested = 4,
    isLocked = 8,
    isEmergency = 16
};

enum Scale {
    scale2 = 0, scale1, scale0, scaleS, scaleH0, scaleTT, scaleN, scaleZ
};

//forward declaration
QPixmap getFunctionPixmap(int type, bool active);
class RoutesWindow;

class LocoControl : public QFrame
{
    Q_OBJECT

    int id;
    bool modified;
    unsigned int srcpstate;
    unsigned int address;
    unsigned int srcpbus;
    unsigned int commandsessionid;
    unsigned int infosessionid;
    unsigned int gmreplyid;
    unsigned int currentroute;
    unsigned int nextroute;
    bool keeplimit;
    bool followlimit;
    int scale;
    float scaledivisor;
    bool keypressed;
    int decoderSpeedSteps;
    int currentSpeedStep;
    int previousSpeedStep;
    int targetSpeedStep;
    int minSpeedStep;
    int avgSpeedStep;
    int maxSpeedStep;
    int maximumSpeed;

    QString locoName;
    unsigned int trainnumber;
    QString locoPicture;
    QString locoProtocol;
    QString locoDecoder;
    QString today, lastUsed;
    unsigned int operationTime;
    unsigned int mfxUid;

    int   showloconame;
    QList<Function> functions;
    QList<Route> routes;
    unsigned int drivemode;    // (0: reverse, 1: forward, 2: emerg. halt
    bool  reverseDirection;    // reverse direction of button arrow
    int   iLocoShowSpeed;
    QString locoSpeedUnit;
    float fLocoSpeedFactor;
    int   iLocoAccelTime;
    int   iLocoBreakTime;
    bool  directionChanged;
    bool  powerOn;

    // parameters for GlMessage
    /*SrcpMessage::Type glmtype;
    GlMessage::Protocol glmprot;
    unsigned int glmprotv;
    unsigned int glmdrivemode;
    unsigned int glmfcount;
    GlMessage glm;*/
    unsigned int glmfmap;
    RoutesViewOptions rvo;

    QAction      *lockAction;
    QPushButton  *m1;
    QPushButton  *m2;
    QPushButton  *m3;
    QPushButton  *buttF[maxfbuttoncount];
    QPushButton  *buttBreak;
    QPushButton  *buttDirection;
    QPushButton  *buttEmergency;
    QPushButton  *buttName;
    QPushButton  *buttSpeed;
    QSlider      *speedSlider;
    QLabel       *lockLabel;
    QLabel       *labelAddress;
    QLabel       *labelPicture;
    QPixmap       locoPixmap;
    QPixmap       directionPixmap;
    QMenu        *contextmenu;
    QButtonGroup *m_group;
    QButtonGroup *groupFuncs;
    QTimer	 *delayTimer;
    QTimer	 *operationTimer;
    QTimer	 *haltTimer;
    RoutesWindow *routesWindow;

    int getDecoderSpeedSteps(const QString&);
    int getDecoderFunctionCount(const QString&);
    void handleFuncKeyPress(int);
    void handleFuncKeyRelease(int);
    void initLayout();
    void readFileTextFromStream(QTextStream&);
//    void sendLocoState();
    void setDefaultValues();
    void updateDirection();
    void updateDisplay();
    void updateSpeedButtonText();
    void setFuButton(QAbstractButton*, int, bool);
//    int getFunctionTypeId(const QString&);
    int getSrcpGlProtocol(const QString&, GlMessage::Protocol&, unsigned int&);
    void updateFunctionStates(unsigned int);
    void changeFunctionState(int);
    void checkForNextRoute(Route*);
    void handleGlInfoMessage(const GlMessage*);
    void handleGmInfoMessage(const GmMessage*);
    void handleCrcfRouteMessage(const CrcfMessage*);
    void handleCrcfRouteStateMessage(const CrcfMessage*);
    void handleCrcfRouteTrainMessage(const CrcfMessage*);
    void handleCrcfRouteNameMessage(const CrcfMessage*);
    void handleCrcfRouteLengthMessage(const CrcfMessage*);
    void handleCrcfRouteSpeedLimitMessage(const CrcfMessage*);
    void handleCrcfRouteEntrySignalAspectMessage(const CrcfMessage*);
    void handleCrcfRouteExitSignalAspectMessage(const CrcfMessage*);
    void handleCrcfTrainMessage(const CrcfMessage*);
    void handleCrcfTrainDirectionMessage(const CrcfMessage*);
    Route* getRouteById(unsigned int);
    void updateDriveAspect();
    void updateDirectionAspect();
    void setCurrentRoute(unsigned int);
    void setNextRoute(unsigned int);
    void handleRouteUpdate();
    void setAutoSpeedStep(int);
    void updateLocoName();
    void setRouteName(Route&, const QString&);
    void setRouteLength(Route&, unsigned int);
    void setRouteSpeedLimit(Route&, unsigned int);
    void setRouteEntrySignalAspect(Route&, unsigned int);
    void setRouteExitSignalAspect(Route&, unsigned int);
    void keepSpeedLimit(int);
    void followSpeedLimit(int);
    void setScale(int);
    void handleHp0(unsigned int);
    void completeRouteData(unsigned int);
    void completeRouteData(Route*);
    void functionButtonTouched(int);

public:
    LocoControl(QTextStream&, int, unsigned int, QWidget* parent = 0);
    LocoControl(int, unsigned int, QWidget* parent = 0);

    void writeConfig(QTextStream&);
    bool isModified();
    bool hasId(int);
    int getId();
    QString getListText() const;
    QString getLastUsed();
    QString getOperationTime();
    QPixmap	getLocoPixmap();
    void sendLocoState();
    void reinitialize();
    void initialize();
    void setInitialized();
    bool initialized();
    bool emergencyHalted();

protected:
    void mousePressEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);

private slots:
    void slotUpdateFuncsPressed (int);
    void slotUpdateFuncsReleased(int);
    void slotUpdateSpeed(int);
    void slotReverseDirection();
    void slotSetAutoSpeed(int);
    void slotToggleNameAlias();
    void slotToggleSpeedDisplay();
    void runPropertiesDialog();
    void slotSpeedTick();
    void slotOperTick();
    void slotDelete();
    void toggleControllerLock();
    void editRoutesViewOptions();
    void executeHalt();

public slots:
	void slotBindRequested();
    void showRoutes();
    void clearRoutes();

signals:
    void closed(int);
    void sendSrcpMessage(const SrcpMessage& sm);
    void statusMessage(const QString&);
    void routeDataChanged(const Route&);

public slots:
    void slotEmergencyHalt(bool);
    void processInfoMessage(const SrcpMessage* sm);
    void processCommandMessage(const SrcpMessage* sm);
    void updateSrcpBus(unsigned int);
    void updateScale(int);

};

#endif  //LOCOCONTROL_H
