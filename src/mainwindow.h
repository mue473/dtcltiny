// mainwindow.h - adapted for raidtcl project 2018 - 2020 by Rainer Müller

/***************************************************************************
                               MainWindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QLayout>
#include <QFrame>
#include <QList>


// states to control controller initialization
enum controllerState {
    csInit = 0, csData, csNormal, csEmergency, csPowerOff
};

/* forward declarations */
class SrcpMessage;
class InfoPort;
class CommandPort;
class LocoControl;
class MessageHistory;



class MainWindow: public QMainWindow
{
    Q_OBJECT

    /*dynamic data*/
    bool modified;
    bool powerOn;
    bool serverConnected;
    int controllerid;
    QString filename;
    unsigned int state;
    unsigned int ignoreAcks;
    /*file data*/
    bool autoConnect;
    bool autoPowerOn;

    /*user preferences*/
    bool autoLoad;
	bool mfxMaster;
	unsigned int rregCntr;
    int scale;
    QString cAutoloadFile;
    unsigned int srcpBus;
    QString lastDir;
    QStringList recentFiles;

    CommandPort *commandPort;
    InfoPort *infoPort;

    QHBoxLayout *controllerLayout;

    MessageHistory* messageHistory;
    QList<LocoControl*> controllers;

    // menu actions which can be enabled/disabled
    QAction *actionFileSave;
    QAction *actionFileSaveAs;

    QAction *actionViewToolbar;
    QAction *actionViewStatusline;
    QAction *actionViewToggleHistory;

    QAction *actionDaemonConnect;
    QAction *actionDaemonDisconnect;
    QAction *actionDaemonReset;
    QAction *actionDaemonShutdown;
    QAction *actionDaemonInfo;
    QAction *actionDaemonEmergency;
    QAction *actionDaemonPower;
    QAction *actionDaemonSettings;

    QAction *actionLocomotiveAdd;
    QAction *actionLocomotiveShow;
    QAction *actionRoutesShow;
    QAction *actionRoutesClear;

    QAction *actionProgrammerNmra;
    QAction *actionProgrammerUhl;

    QMenu    *fileRecentlyOpenedFiles;

    void setupMainWindow();
    void handleRregRequest();
    void loadPreferences();
    void savePreferences();
    void updateCaption();
    void updateDaemonMenuItems();
    void updatePowerMenuItems();
    void newFile();
    bool saveFile();
    void chooseFile();
    bool isModified();
    bool isSave();
    int querySaveChanges();
    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);

public:
    MainWindow();
    ~MainWindow();
    void openFile(const QString&);
    void openFileWindow(const QString&);
    void readAutoloadFile();

protected:
    void closeEvent(QCloseEvent *e);

public slots:
    void processCommandMessage(const SrcpMessage*);
    void processInfoMessage(const SrcpMessage*);
    void updateConnectionState(bool);
    void updatePowerState(int);

private slots:
    //void sendSrcpMessageString(const QString&);
    void sendSrcpMessage(const SrcpMessage& sm);
    void slotDaemonConnect();
    void slotDaemonDisconnect();
    void slotDaemonEmergency();
    void slotDaemonInfo();
    void slotDaemonPower();
    void slotDaemonReset();
    void slotDaemonShutdown();
    void slotFileNew();
    void slotFileOpen();
    void slotFileSave();
    void slotFileSaveAs();
    void slotHelpAbout();
    void slotHelpAboutQt();
    void slotHelpHelp();
    void slotLocomotiveAdd();
    void slotLocomotiveShow();
    void slotRoutesShow();
    void slotRoutesClear();
    void slotLocomotiveRemove(int);
    void slotDaemonSettings();
    void slotOptionsPreferences();
    void slotProgramNMRA();
    void slotProgramUhl();
    void slotShowController(int);
    void recentFileActivated(QAction*);
    void setupRecentFilesMenu();

signals:
    void statusMessage(const QString&);
    void srcpBusChanged(unsigned int);
    void scaleChanged(int);
};

#endif  //MAINWINDOW_H
