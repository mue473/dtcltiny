// mainwindow.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
                               MainWindow.cpp
                             -------------------
    begin                : 11.11.2000
    last update		 : 2008-12-15
    copyright            : (C) 2000 by Markus Pfeiffer
                           (C) 2004-2008 Guido Scholz
    email                : dtcltiny@markus-pfeiffer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCloseEvent>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QScrollArea>
#include <QTextStream>
#include <QToolBar>
#include <QDesktopServices>

#include "config.h"
#include "mainwindow.h"
#include "aboutdialog.h"
#include "hiddencontrollersdialog.h"
#include "listpropertiesdialog.h"
#include "commandport.h"
#include "infoport.h"
#include "lococontrol.h"
#include "messagehistory.h"
#include "locodialog.h"
#include "programmer.h"
#include "preferencesdlg.h"
#include "serverinfodialog.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    #define endl Qt::endl
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    #define SETUTF8 ts.setEncoding(QStringConverter::Utf8)
#else
    #define SETUTF8 ts.setCodec("utf-8");
#endif

//constants for data file
static const char KS[]             = "=";
static const char DS[]             = ":";
static const char DF_HOST[]        = "host";
static const char DF_SRCPBUS[]     = "srcpbus";
static const char DF_AUTOCONNECT[] = "autoconnect";
static const char DF_AUTOPOWERON[] = "autopoweron";

//constants for user preferences file
static const char PF_SHOWMSGTIME[] =   "showmessagetime";
static const char PF_TRANSLSRVTIME[] = "translateservertime";
static const char PF_AUTOLOAD[] =      "autoload";
static const char PF_AUTOLOADFILE[] =  "autoloadfile";
static const char PF_MFXMASTER[] =	   "mfxmaster"; 
static const char PF_RREGCOUNTER[] =   "rregcounter"; 
static const char PF_SCALE[] =         "scale";
static const char PF_SRCPBUS[] =       "srcpbus";
static const char PF_LASTDIR[] =       "lastdir";
static const char PF_SHOWTOOLBAR[] =   "showtoolbar";
static const char PF_SHOWSTATUSBAR[] = "showstatusbar";
static const char CF_RECENTFILE[] =    "recentfile";

/* resource file */
static const char DTCLTINYRC[] = ".dtcltinyrc";


/* pixmaps for icons */
#include "../icons/dtcltiny_32.xpm"

// file menu
#include "pixmaps/filenew.xpm"
#include "pixmaps/fileopen.xpm"
#include "pixmaps/filesave.xpm"

// view menu
#include "pixmaps/viewtoolbar.xpm"
#include "pixmaps/viewstatusline.xpm"
#include "pixmaps/viewtogglehistory.xpm"

// daemon menu
#include "pixmaps/daemonconnect.xpm"
#include "pixmaps/daemondisconnect.xpm"
#include "pixmaps/daemonreset.xpm"
#include "pixmaps/daemonshutdown.xpm"
#include "pixmaps/daemoninfo.xpm"
#include "pixmaps/startpower.xpm"
#include "pixmaps/stoppower.xpm"
#include "pixmaps/emergency_all.xpm"
#include "pixmaps/daemonsettings.xpm"

// loco menu
#include "pixmaps/newloco.xpm"
#include "pixmaps/controllershow.xpm"
#include "pixmaps/routesshow.xpm"
#include "pixmaps/routesclear.xpm"

// programmer menu
#include "pixmaps/programmernmra.xpm"
#include "pixmaps/programmeruhl.xpm"



MainWindow::MainWindow(): QMainWindow(NULL),
    modified(false),
    powerOn(false),
    serverConnected(false),
    controllerid(0),
    filename(""),
    autoConnect(false),
    autoPowerOn(false),
    autoLoad(false),
    mfxMaster(false),
	rregCntr(0),
    scale(scaleH0),
    cAutoloadFile(""),
    srcpBus(1)
{
    QWidget::setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(PACKAGE);
    setWindowIcon(QPixmap(dtcltiny_32));

    lastDir = QDir::homePath();

    // init sockets with default values
    commandPort = new CommandPort(this, "commandPort", "localhost", 4303);
    Q_CHECK_PTR(commandPort);

    // receive connection state changes
    connect(commandPort, SIGNAL(connectionStateChanged(bool)),
            this, SLOT(updateConnectionState(bool)));
    connect(commandPort, SIGNAL(messageReceived(const SrcpMessage*)),
            this, SLOT(processCommandMessage(const SrcpMessage*)));

    infoPort = new InfoPort(this, "infoPort", "localhost", 4303);
    Q_CHECK_PTR(infoPort);

    // receive info messages to detect power changes etc.
    connect(infoPort, SIGNAL(messageReceived(const SrcpMessage*)),
            this, SLOT(processInfoMessage(const SrcpMessage*)));

    // setup application's main window
    setupMainWindow();
    loadPreferences();

    updateDaemonMenuItems();
}


MainWindow::~MainWindow()
{
    slotDaemonDisconnect();
    savePreferences();
}


void MainWindow::setupMainWindow()
{
    QVBoxLayout *baseLayout = new QVBoxLayout();
    baseLayout->setSpacing(0);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *vBox = new QWidget(this);
    Q_CHECK_PTR(vBox);
    vBox->setLayout(baseLayout);

    // create container layout for locomotive controllers
    controllerLayout = new QHBoxLayout();
    controllerLayout->setSpacing(0);
    controllerLayout->setContentsMargins(0, 0, 0, 0);
    controllerLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // setup the background widget for the controllers
    QWidget* controllerArea = new QWidget();
    QPalette p(controllerArea->palette());
    p.setColor(controllerArea->backgroundRole(), Qt::darkGray);
    controllerArea->setPalette(p);
    controllerArea->setLayout(controllerLayout);

    QScrollArea* sv = new QScrollArea();
    Q_CHECK_PTR(sv);
    sv->setWidgetResizable(true);
    sv->setWidget(controllerArea);
    baseLayout->addWidget(sv);

    //setup message history line
    messageHistory = new MessageHistory();
    Q_CHECK_PTR(messageHistory);
    baseLayout->addWidget(messageHistory);

    setCentralWidget(vBox);

    connect(commandPort, SIGNAL(messageReceived(const QString&)),
            messageHistory, SLOT(addCommand(const QString&)));
    connect(commandPort, SIGNAL(messageSend(const QString&)),
            messageHistory, SLOT(addCommand(const QString&)));
    connect(commandPort, SIGNAL(statusMessage(const QString&)),
            messageHistory, SLOT(addHint(const QString&)));

    connect(infoPort, SIGNAL(messageReceived(const QString&)),
            messageHistory, SLOT(addInfo(const QString&)));
    connect(infoPort, SIGNAL(messageSend(const QString&)),
            messageHistory, SLOT(addInfo(const QString&)));
    connect(infoPort, SIGNAL(statusMessage(const QString&)),
            messageHistory, SLOT(addHint(const QString&)));

    connect(this, SIGNAL(statusMessage(const QString&)),
            messageHistory, SLOT(addHint(const QString&)));

    /*file toolbar*/
    QToolBar* filetb = new QToolBar(tr("File operations"), this);
    Q_CHECK_PTR(filetb);
    addToolBar(Qt::TopToolBarArea, filetb);

    /*create main menu */
    QMenu* filemenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(filemenu);

    /*file actions*/
    QAction *actionFileNew = new QAction(
            QIcon::fromTheme("document-new", QPixmap(filenew)),
            tr("&New..."), this);
    actionFileNew->setShortcut(QKeySequence::New);

    actionFileNew->setToolTip(tr("Create new file"));
    connect(actionFileNew, SIGNAL(triggered()), this, SLOT(slotFileNew()));
    filemenu->addAction(actionFileNew);
    filetb->addAction(actionFileNew);

    QAction *actionFileOpen = new QAction(
            QIcon::fromTheme("document-open", QPixmap(fileopen_xpm)),
            tr("&Open..."), this);
    actionFileOpen->setShortcut(QKeySequence::Open);
    actionFileOpen->setToolTip(tr("Open loco controller file"));
    connect(actionFileOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()));
    filemenu->addAction(actionFileOpen);
    filetb->addAction(actionFileOpen);

    fileRecentlyOpenedFiles = new QMenu(tr("&Recently opened files"), this);
    fileRecentlyOpenedFiles->setIcon(QIcon::fromTheme("document-open-recent"));
    filemenu->addMenu(fileRecentlyOpenedFiles);

    actionFileSave = new QAction(
            QIcon::fromTheme("document-save", QPixmap(filesave_xpm)),
            tr("&Save"), this);
    actionFileSave->setShortcut(QKeySequence::Save);
    actionFileSave->setToolTip(tr("Save loco controller file"));
    connect(actionFileSave, SIGNAL(triggered()), this, SLOT(slotFileSave()));
    filemenu->addAction(actionFileSave);
    filetb->addAction(actionFileSave);

    actionFileSaveAs = new QAction(QIcon::fromTheme("document-save-as"),
                tr("Save &as..."), this);
    actionFileSaveAs->setShortcut(Qt::CTRL | Qt::Key_A);
    actionFileSaveAs->setToolTip(tr("Save loco controllers to new file"));
    connect(actionFileSaveAs, SIGNAL(triggered()), this,
            SLOT(slotFileSaveAs()));
    filemenu->addAction(actionFileSaveAs);

    filemenu->addSeparator();
    filetb->addSeparator();

    QAction *actionFileQuit = new QAction(
            QIcon::fromTheme("window-close"),
            tr("&Quit"), this);
    actionFileQuit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(actionFileQuit, SIGNAL(triggered()), qApp,
            SLOT(closeAllWindows()));
    filemenu->addAction(actionFileQuit);

    // when file menu is complete connect action
    connect(filemenu, SIGNAL(aboutToShow()), this,
            SLOT(setupRecentFilesMenu()));
    connect(fileRecentlyOpenedFiles, SIGNAL(triggered(QAction*)), this,
            SLOT(recentFileActivated(QAction*)));

    /*view toolbar*/
    QToolBar* viewtb = new QToolBar(tr("View operations"), this);
    Q_CHECK_PTR(viewtb);
    addToolBar(Qt::TopToolBarArea, viewtb);

    /*view menu*/
    QMenu* viewmenu = new QMenu(tr("&View"), this);
    menuBar()->addMenu(viewmenu);

    actionViewToolbar = new QAction(QPixmap(viewtoolbar_xpm),
            tr("&Toolbars"), this);
    actionViewToolbar->setToolTip(tr("Show/hide toolbars"));
    actionViewToolbar->setCheckable(true);
    actionViewToolbar->setChecked(true);
    viewmenu->addAction(actionViewToolbar);
    viewtb->addAction(actionViewToolbar);
    /*add action to mainwindow to keep * responsiveness if menu is hidden*/
    addAction(actionViewToolbar);


    actionViewStatusline = new QAction(QPixmap(viewstatusline_xpm),
            tr("&Statusline"), this);
    actionViewStatusline->setToolTip(tr("Show/hide statusline"));
    actionViewStatusline->setCheckable(true);
    actionViewStatusline->setChecked(true);
    connect(actionViewStatusline, SIGNAL(toggled(bool)), messageHistory,
            SLOT(setVisible(bool)));
    viewmenu->addAction(actionViewStatusline);
    viewtb->addAction(actionViewStatusline);
    /*add action to mainwindow to keep responsiveness if menu is hidden*/
    addAction(actionViewStatusline);


    actionViewToggleHistory = new QAction(QPixmap(viewtogglehistory_xpm),
            tr("Toggle &history line"), this);
    actionViewToggleHistory->setShortcut(Qt::CTRL | Qt::Key_H);
    actionViewToggleHistory->setToolTip(tr("Toggle history line"));
    connect(actionViewToggleHistory, SIGNAL(triggered()), messageHistory,
            SLOT(toggleLine()));
    viewmenu->addAction(actionViewToggleHistory);
    viewtb->addAction(actionViewToggleHistory);


    /*daemon toolbar*/
    QToolBar* daemontb = new QToolBar(tr("SRCP-Server operations"), this);
    Q_CHECK_PTR(daemontb);
    addToolBar(Qt::TopToolBarArea, daemontb);

    QMenu* daemonmenu = new QMenu(tr("&SRCP-Server"), this);
    menuBar()->addMenu(daemonmenu);

    actionDaemonConnect = new QAction(QPixmap(daemonconnect_xpm),
            tr("&Connect"), this);
    //actionDaemonConnect->setShortcut(Qt::Key_F2);
    actionDaemonConnect->setToolTip(tr("Connect to SRCP server"));
    connect(actionDaemonConnect, SIGNAL(triggered()), this,
            SLOT(slotDaemonConnect()));
    daemonmenu->addAction(actionDaemonConnect);
    daemontb->addAction(actionDaemonConnect);

    actionDaemonDisconnect = new QAction(QPixmap(daemondisconnect_xpm),
            tr("&Disconnect"), this);
    //actionDaemonDisconnect->setShortcut(Qt::Key_F3);
    actionDaemonDisconnect->setToolTip(tr("Disconnect SRCP server"));
    connect(actionDaemonDisconnect, SIGNAL(triggered()), this,
            SLOT(slotDaemonDisconnect()));
    daemonmenu->addAction(actionDaemonDisconnect);
    daemontb->addAction(actionDaemonDisconnect);

    daemonmenu->addSeparator();
    daemontb->addSeparator();

    actionDaemonReset = new QAction(QPixmap(daemonreset_xpm),
            tr("&Reset"), this);
    actionDaemonReset->setToolTip(tr("Reset SRCP server"));
    connect(actionDaemonReset, SIGNAL(triggered()), this,
            SLOT(slotDaemonReset()));
    daemonmenu->addAction(actionDaemonReset);
    daemontb->addAction(actionDaemonReset);

    actionDaemonShutdown = new QAction(QPixmap(daemonshutdown_xpm),
            tr("&Shutdown"), this);
    actionDaemonShutdown->setToolTip(tr("Shutdown SRCP server"));
    connect(actionDaemonShutdown, SIGNAL(triggered()), this,
            SLOT(slotDaemonShutdown()));
    daemonmenu->addAction(actionDaemonShutdown);
    daemontb->addAction(actionDaemonShutdown);

    actionDaemonInfo = new QAction(QPixmap(daemoninfo_xpm),
            tr("&Info..."), this);
    actionDaemonInfo->setToolTip(tr("Show SRCP server information"));
    connect(actionDaemonInfo, SIGNAL(triggered()), this,
            SLOT(slotDaemonInfo()));
    daemonmenu->addAction(actionDaemonInfo);
    daemontb->addAction(actionDaemonInfo);

    daemonmenu->addSeparator();
    daemontb->addSeparator();

    actionDaemonPower = new QAction(QPixmap(startpower),
            tr("&Power on/off"), this);
    actionDaemonPower->setShortcut(Qt::CTRL | Qt::Key_P);
    actionDaemonPower->setToolTip(tr("Switch power on/off"));
    connect(actionDaemonPower, SIGNAL(triggered()), this,
            SLOT(slotDaemonPower()));
    daemonmenu->addAction(actionDaemonPower);
    daemontb->addAction(actionDaemonPower);

    actionDaemonEmergency= new QAction(QPixmap(emergency_all),
            tr("&Emergengy stop"), this);
    actionDaemonEmergency->setShortcut(Qt::CTRL | Qt::Key_E);
    actionDaemonEmergency->setToolTip(tr("Activate emergengy stop"));
    connect(actionDaemonEmergency, SIGNAL(triggered()), this,
            SLOT(slotDaemonEmergency()));
    daemonmenu->addAction(actionDaemonEmergency);
    daemontb->addAction(actionDaemonEmergency);

    daemonmenu->addSeparator();
    daemontb->addSeparator();

    actionDaemonSettings = new QAction(QPixmap(daemonsettings_xpm),
            tr("Ser&ver settings..."), this);
    //actionDaemonSettings->setShortcut(Qt::CTRL + Qt::Key_L);
    actionDaemonSettings->setToolTip(tr("Edit server settings"));
    connect(actionDaemonSettings, SIGNAL(triggered()), this,
            SLOT(slotDaemonSettings()));
    daemonmenu->addAction(actionDaemonSettings);
    daemontb->addAction(actionDaemonSettings);


    /*locomotive toolbar*/
    QToolBar* locomotivetb = new QToolBar(
            tr("Locomotive controller operations"), this);
    Q_CHECK_PTR(locomotivetb);
    addToolBar(Qt::TopToolBarArea, locomotivetb);

    QMenu* locomotivemenu = new QMenu(tr("&Controller"), this);
    menuBar()->addMenu(locomotivemenu);

    actionLocomotiveAdd = new QAction(QPixmap(newloco),
            tr("&Add"), this);
    //actionLocomotiveAdd->setShortcut(Qt::Key_F4);
    actionLocomotiveAdd->setToolTip(tr("Add new locomotive controller"));
    connect(actionLocomotiveAdd, SIGNAL(triggered()), this,
            SLOT(slotLocomotiveAdd()));
    locomotivemenu->addAction(actionLocomotiveAdd);
    locomotivetb->addAction(actionLocomotiveAdd);

    actionLocomotiveShow = new QAction(QPixmap(controllershow_xpm),
            tr("&Show..."), this);
    //actionLocomotiveAdd->setShortcut(Qt::Key_F4);
    actionLocomotiveShow->setToolTip(
            tr("Show hidden locomotive controllers"));
    connect(actionLocomotiveShow, SIGNAL(triggered()), this,
            SLOT(slotLocomotiveShow()));
    locomotivemenu->addAction(actionLocomotiveShow);
    locomotivetb->addAction(actionLocomotiveShow);

    actionRoutesShow = new QAction(QPixmap(routesshow_xpm),
            tr("&Show all routes"), this);
    //actionRoutesShow->setShortcut(Qt::Key_F4);
    actionRoutesShow->setToolTip(
            tr("Show routes windows of all visible controllers"));
    connect(actionRoutesShow, SIGNAL(triggered()), this,
            SLOT(slotRoutesShow()));
    locomotivemenu->addAction(actionRoutesShow);
    locomotivetb->addAction(actionRoutesShow);

    actionRoutesClear = new QAction(QPixmap(routesclear_xpm),
            tr("&Clear all routes"), this);
    //actionRoutesClear->setShortcut(Qt::Key_F4);
    actionRoutesClear->setToolTip(
            tr("Clear all routes of all controllers"));
    connect(actionRoutesClear, SIGNAL(triggered()), this,
            SLOT(slotRoutesClear()));
    locomotivemenu->addAction(actionRoutesClear);
    locomotivetb->addAction(actionRoutesClear);


    /*programmer toolbar*/
    QToolBar* programmertb = new QToolBar(tr("Programmer operations"), this);
    Q_CHECK_PTR(programmertb);
    addToolBar(Qt::TopToolBarArea, programmertb);

    QMenu* programmermenu = new QMenu(tr("&Programmer"), this);
    menuBar()->addMenu(programmermenu);

    actionProgrammerNmra = new QAction(QPixmap(programmernmra_xpm),
            tr("&NMRA-Decoder..."), this);
    //actionProgrammerNmra->setShortcut(Qt::Key_F5);
    actionProgrammerNmra->setToolTip(tr("Show NMRA-Programmer"));
    connect(actionProgrammerNmra, SIGNAL(triggered()), this,
            SLOT(slotProgramNMRA()));
    programmermenu->addAction(actionProgrammerNmra);
    programmertb->addAction(actionProgrammerNmra);

    actionProgrammerUhl = new QAction(QPixmap(programmeruhl_xpm),
            tr("&Uhlenbrock-Decoder..."), this);
    //actionProgrammerUhl->setShortcut(Qt::Key_F6);
    actionProgrammerUhl->setToolTip(tr("Show Uhlenbrock-Programmer"));
    connect(actionProgrammerUhl, SIGNAL(triggered()), this,
            SLOT(slotProgramUhl()));
    programmermenu->addAction(actionProgrammerUhl);
    programmertb->addAction(actionProgrammerUhl);

    /*no options toolbar*/
    /*options menu*/
    QMenu* optionsmenu = new QMenu(tr("&Options"), this);
    menuBar()->addMenu(optionsmenu);

    QAction *actionOptionsPrefs = new QAction(
            tr("&Preferences..."), this);
    //actionOptionsPrefs->setShortcut(Qt::CTRL + Qt::Key_U);
    actionOptionsPrefs->setToolTip(tr("Edit program preferences"));
    connect(actionOptionsPrefs, SIGNAL(triggered()), this,
            SLOT(slotOptionsPreferences()));
    optionsmenu->addAction(actionOptionsPrefs);

    /*help menu*/
    QMenu* helpmenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpmenu);

    QAction *actionHelp = new QAction(tr("&Help"), this);
    actionHelp->setShortcut(Qt::CTRL | Qt::Key_F1);
    connect(actionHelp, SIGNAL(triggered()), this,
            SLOT(slotHelpHelp()));
    helpmenu->addAction(actionHelp);
    helpmenu->addAction(tr("&About %1...").arg(PACKAGE),
            this, SLOT(slotHelpAbout()));
    helpmenu->addAction(tr("About &Qt..."), this, SLOT(slotHelpAboutQt()));

    //at least connect all toolbars to switch visibility
    connect(actionViewToolbar, SIGNAL(toggled(bool)), filetb,
            SLOT(setVisible(bool)));
    connect(actionViewToolbar, SIGNAL(toggled(bool)), viewtb,
            SLOT(setVisible(bool)));
    connect(actionViewToolbar, SIGNAL(toggled(bool)), daemontb,
            SLOT(setVisible(bool)));
    connect(actionViewToolbar, SIGNAL(toggled(bool)), locomotivetb,
            SLOT(setVisible(bool)));
    connect(actionViewToolbar, SIGNAL(toggled(bool)), programmertb,
            SLOT(setVisible(bool)));
}


/*
 * create a new file
 */
void MainWindow::slotFileNew()
{
    if (isSave())
        newFile();
}


int MainWindow::querySaveChanges()
{
    QString queryStr;

    if (filename.isEmpty())
        queryStr = tr("Unnamed file was changed.\nSave changes?");
    else
        queryStr = tr("File '%1' was changed.\n"
                "Save changes?").arg(filename);

    return QMessageBox::warning(this, tr("Save changes"), queryStr,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}


void MainWindow::newFile()
{
    slotDaemonDisconnect();

    // reset dynamic data
    serverConnected = false;
    powerOn = false;
    modified = false;
    filename = "";

    // reset file data
    commandPort->setServer("localhost", 4303);
    infoPort->setServer("localhost", 4303);
    autoConnect = false;
    autoPowerOn = false;


    // delete all current controllers
    while (!controllers.isEmpty())
        delete controllers.takeFirst();

    updateCaption();
    //updateFileMenuItems();
    emit statusMessage(tr("New file created."));
}


void MainWindow::updateCaption()
{
    if (filename.isEmpty())
        setWindowTitle(QString(PACKAGE) + " - [" + tr("noname") + "]");
    else
        setWindowTitle(QString(PACKAGE) + " - [" + filename + "]");
}

/*check for changed file data*/
bool MainWindow::isModified()
{
    for (int i = 0; i < controllers.size(); ++i) {
        if (controllers.at(i)->isModified())
            return true;
    }
    return modified;
}


void MainWindow::slotFileOpen()
{
    if (isSave())
        chooseFile();
}


void MainWindow::chooseFile()
{
    QStringList fnl = QFileDialog::getOpenFileNames(this,
        tr("Select controller configuration"), lastDir,
		QString(tr("Loco sets")) + " (*" + DTCLFILEEXT + ")", 
						NULL, QFileDialog::DontUseNativeDialog);

    if (fnl.isEmpty())
        return;
    else {
        openFile(fnl.first());
        fnl.pop_front();

        QStringList::Iterator it = fnl.begin();
        while (it != fnl.end()) {
                openFileWindow(*it);
                ++it;
        }
    }
}


void MainWindow::slotFileSave()
{
    saveFile();
}


bool MainWindow::saveFile()
{
    if (filename.isEmpty()){
        slotFileSaveAs();
        return true;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly) == false) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not open %1 for writing.").
                             arg(filename));
        return false;
    }

    QTextStream ts(&file);
    SETUTF8;
    QDateTime dt = QDateTime::currentDateTime();

    // write the header
    ts << "# dtcltiny data file" << endl
        << "# version=" << VERSION << endl
        << "# last modified=" << dt.toString(Qt::ISODate) << endl
        << DF_HOST << KS << commandPort->getHostname()
        << DS << commandPort->getPortNumber() << endl
        << DF_SRCPBUS << KS << srcpBus << endl
        << DF_AUTOCONNECT << KS << autoConnect << endl
        << DF_AUTOPOWERON << KS << autoPowerOn << endl
        << "%" << endl
        << "# end of header section" << endl;


    for (int i = 0; i < controllers.size(); ++i) {
        controllers.at(i)->writeConfig(ts);
    }

    file.close();
    updateCaption();
    emit statusMessage(tr("File %1 saved.").arg(filename));
    modified = false;

    return true;
}


void MainWindow::slotFileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this,
            tr("Save controller configuration"), lastDir,
            QString(tr("Loco sets")) + " (*" + DTCLFILEEXT + ")", 
						NULL, QFileDialog::DontUseNativeDialog);

    if (!fn.isEmpty()) {

        lastDir = fn.left(fn.lastIndexOf('/'));

        /*check for file extension*/
        if (!fn.endsWith(DTCLFILEEXT))
            fn.append(DTCLFILEEXT);

        filename = fn;
        saveFile();
    }
}


bool MainWindow::isSave()
{
    bool result = false;

    if (isModified()) {
        int choice = querySaveChanges();
        switch (choice) {
            case QMessageBox::Yes:
                if (saveFile())
                    result = true;
                break;
            case QMessageBox::No:
                result = true;
                break;
            case QMessageBox::Cancel:
            default:
                break;
        }
    }
    else
        result = true;
    return result;
}


void MainWindow::openFile(const QString& fn)
{
    lastDir = fn.left(fn.lastIndexOf('/'));

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
        emit statusMessage(tr("Error, could not read file '%1'.").arg(fn));
        return;
    }

    slotDaemonDisconnect();
    qApp->processEvents();

    filename = fn;
    addRecentlyOpenedFile(fn, recentFiles);

    // delete all current controllers
    while (!controllers.isEmpty())
        delete controllers.takeFirst();

    QTextStream ts(&file);
    SETUTF8;

    QString s, key, value;

    while (!ts.atEnd()) {
        s = ts.readLine();
        key = s.section(KS, 0, 0);
        value = s.section(KS, 1, 1);

        /* ignore comment lines */
        if (s.startsWith("#"))
            continue;

        else if (key.compare(DF_HOST) == 0) {
            QStringList tokens = value.split(DS);
            commandPort->setServer(tokens[0], tokens[1].toUInt());
            infoPort->setServer(tokens[0], tokens[1].toUInt());
        }
        else if (key.compare(DF_SRCPBUS) == 0) {
             srcpBus = value.toUInt();
        }
        else if (key.compare(DF_AUTOCONNECT) == 0) {
             autoConnect = value.toInt();
        }
        else if (key.compare(DF_AUTOPOWERON) == 0) {
             autoPowerOn = value.toInt();
        }

        // check for visibility information:
        // LOCO <id> <hidden>
        //  0    1      2
        else if (s.startsWith("LOCO")) {
            QStringList tokens = s.split(' ');
            bool ishidden = (tokens.count() > 2 && tokens[2] == "1");

            LocoControl* lc = new LocoControl(ts, controllerid, srcpBus, NULL);
            if (lc != NULL) {
                connect(lc, SIGNAL(closed(int)),
                        this, SLOT(slotLocomotiveRemove(int)));
                connect(lc, SIGNAL(sendSrcpMessage(const SrcpMessage&)),
                        this, SLOT(sendSrcpMessage(const SrcpMessage&)));
                connect(this, SIGNAL(srcpBusChanged(unsigned int)),
                        lc, SLOT(updateSrcpBus(unsigned int)));
                connect(this, SIGNAL(scaleChanged(int)),
                        lc, SLOT(updateScale(int)));
                connect(commandPort, SIGNAL(
                            messageReceived(const SrcpMessage*)),
                        lc, SLOT(
                            processCommandMessage(const SrcpMessage*)));
                connect(infoPort, SIGNAL(
                            messageReceived(const SrcpMessage*)),
                        lc, SLOT(processInfoMessage(const SrcpMessage*)));
                connect(lc, SIGNAL(statusMessage(const QString&)),
                        messageHistory, SLOT(addHint(const QString&)));

                controllerid++;
                controllers.append(lc);
                controllerLayout->addWidget(lc);
                if (ishidden)
                    lc->hide();
                else
                    lc->show();
                qApp->processEvents();
            }
            else
                emit statusMessage(
                        tr("Error: Could not create new locomotive"
                            " controller"));
        }

    }

    file.close();
    emit statusMessage(tr("File with %1 locos loaded: %2")
            .arg(controllers.count()).arg(fn));

    //TODO: updateLocoNumberSensibleItems();
    if (controllers.count() > 0) {
        actionFileSave->setEnabled(true);
        updatePowerMenuItems();
    }
    else {
        actionFileSave->setEnabled(false);
        updatePowerMenuItems();
    }
    updateCaption();

    if (autoConnect)
        slotDaemonConnect();
}


/*
 * show response to server (command session) connection state changes
 */
void MainWindow::updateConnectionState(bool connected)
{
    if (serverConnected != connected) {
        serverConnected = connected;
        updateDaemonMenuItems();
        if (serverConnected) {
        	ignoreAcks = 0;
        	handleRregRequest();
            infoPort->setServer(commandPort->getHostname(),
                    commandPort->getPortNumber());
            infoPort->serverConnect();
        }
        // reset controllers to be initialized again
        else
            for (int i = 0; i < controllers.size(); ++i) {
                LocoControl * lc = controllers.at(i);
                lc->reinitialize();
            }
    }
}

/*
 * show response to layout power state changes
 */
void MainWindow::updatePowerState(int ison)
{
	if (ison < 0) return;
    if (powerOn != (ison > 0)) {
        powerOn = ison;
        updatePowerMenuItems();
    }
}

/*
 * response to requested power change info message
 */
void MainWindow::processCommandMessage(const SrcpMessage* sm)
{
    switch (sm->getType()) {
        case SrcpMessage::mtAckHandShake:
            // do init actions, save session id; depending on last
            // SRCP action?
            if (SrcpMessage::code200 == sm->getCode()) {
                //qWarning("Command session id: %d",
                //        ((AckMessage*)sm)->getSessionId());
            }
            break;
        case SrcpMessage::mtAckCommand:
        	if (ignoreAcks > 0) {
        		ignoreAcks--;
        		return;
        	}
            if (csData == state) {
                //send first GL SET now
                for (int i = 0; i < controllers.size(); ++i) {
                    LocoControl * lc = controllers.at(i);
                    if (!lc->initialized()) {
                        state = csInit;
                        if (lc == controllers.last()) state = csNormal;
                        lc->setInitialized();
                		//	qWarning("Answer Code: %d", sm->getCode());
            			if (SrcpMessage::code200 == sm->getCode()) {
	                        lc->sendLocoState();
	                        return;		// wait for answer from GL SET
	                    }
                        break;
                    }
                }
            }
            if (csInit == state) {
                //initalize next controller
                for (int i = 0; i < controllers.size(); ++i) {
                    LocoControl * lc = controllers.at(i);
                    if (!lc->initialized()) {
                        lc->initialize();
                        state = csData;
                        break;
                    }
                }
            }
            else if (csEmergency == state) {
                //emergency halt next controller
                for (int i = 0; i < controllers.size(); ++i) {
                    LocoControl * lc = controllers.at(i);
                    if (!lc->emergencyHalted()) {
                        lc->slotEmergencyHalt(true);
                        if (lc == controllers.last()) {
                            state = csPowerOff;
                        }
                        break;
                    }
                }
            }
            else if (csPowerOff == state) {
                slotDaemonPower();
                emit statusMessage(tr("Emergency stop done"));
                state = csNormal;
            }
            break;
        default:
            break;
    }
}


/*
 * filter incoming power change info message
 */
void MainWindow::processInfoMessage(const SrcpMessage* sm)
{
    switch (sm->getType()) {
        case SrcpMessage::mtPowerInfo:
            // external instance (other session) switched power
            updatePowerState(((PowerMessage*)sm)->busison(srcpBus));
            break;
        case SrcpMessage::mtAckHandShake:
            // do init actions, save session id; depending on last
            // SRCP action?
            if (SrcpMessage::code200 == sm->getCode()) {
                //	qWarning("Info session id: %d", ((AckMessage*)sm)->getSessionId());

                //procedure start init sequence
                if (controllers.size() > 0) {
                    LocoControl * lc = controllers.first();
                    if (!lc->initialized()) {
						lc->initialize();
                    	state = csData;
                    }
                }
                else
                    state = csNormal;
            }
            break;
        case SrcpMessage::mtServerInfo:

            switch (((ServerMessage*)sm)->getState()) {
                // reset controllers to be initialized again
                case ServerMessage::ssResetting:
                    for (int i = 0; i < controllers.size(); ++i) {
                        LocoControl * lc = controllers.at(i);
                        lc->reinitialize();
                    }
                    break;
                case ServerMessage::ssRunning:
                	//	qWarning("ServerMessage::ssRunning");
                    //procedure start init sequence (again)
                    if (controllers.size() > 0) {
                        LocoControl * lc = controllers.first();
                    	if (!lc->initialized()) {
							lc->initialize();
                        	state = csData;
                        }
                    }
                    else
                        state = csNormal;
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }
}

void MainWindow::slotLocomotiveAdd()
{
    LocoControl* lc = new LocoControl(controllerid, srcpBus, NULL);
    if (lc == NULL) {
        emit statusMessage(
                tr("Error: Could not create new locomotive controller"));
        return;
    }

    connect(lc, SIGNAL(closed(int)), this,
            SLOT(slotLocomotiveRemove(int)));
    connect(lc, SIGNAL(sendSrcpMessage(const SrcpMessage&)),
            this, SLOT(sendSrcpMessage(const SrcpMessage&)));
    connect(this, SIGNAL(srcpBusChanged(unsigned int)),
            lc, SLOT(updateSrcpBus(unsigned int)));
    connect(this, SIGNAL(scaleChanged(int)),
            lc, SLOT(updateScale(int)));
    connect(commandPort, SIGNAL(messageReceived(const SrcpMessage*)),
            lc, SLOT(processCommandMessage(const SrcpMessage*)));
    connect(infoPort, SIGNAL(messageReceived(const SrcpMessage*)),
            lc, SLOT(processInfoMessage(const SrcpMessage*)));
    connect(lc, SIGNAL(statusMessage(const QString&)),
            messageHistory, SLOT(addHint(const QString&)));

    controllerid++;
    controllers.append(lc);
    controllerLayout->addWidget(lc);
    lc->show();

    if (controllers.count() == 1) {
        updatePowerMenuItems();
    }

    modified = true;
}

// Show dialog to show hidden controllers
void MainWindow::slotLocomotiveShow()
{
    HiddenControllersDialog *dlg = new HiddenControllersDialog(this);

    connect(dlg, SIGNAL(showController(int)),
            this, SLOT(slotShowController(int)));

    dlg->setControllersList(controllers);

    if (dlg->exec() == QDialog::Accepted) {
    }
    delete dlg;
}

// Show routes window of all visible controllers
void MainWindow::slotRoutesShow()
{
    for (int i = 0; i < controllers.size(); ++i) {
        LocoControl *lc = controllers.at(i);
        if (!lc->isHidden())
            lc->showRoutes();
    }
}

// Clear all routes of all controllers
void MainWindow::slotRoutesClear()
{
    for (int i = 0; i < controllers.size(); ++i) {
        controllers.at(i)->clearRoutes();
    }
}


void MainWindow::slotShowController(int cid)
{
    for (int i = 0; i < controllers.size(); ++i) {
        if (controllers.at(i)->hasId(cid)){
            controllers.at(i)->show();
            modified = true;
            break;
        }
    }
}


void MainWindow::slotDaemonSettings()
{
    ListPropertiesDialog *dlg = new ListPropertiesDialog(this);

    dlg->setServerData(commandPort->getHostname(),
            commandPort->getPortNumber());
    dlg->setSrcpBus(srcpBus);
    dlg->setActionData(autoConnect, autoPowerOn);

    if (dlg->exec() == QDialog::Accepted) {
        commandPort->setServer(dlg->getHostname(), dlg->getPort());
        infoPort->setServer(dlg->getHostname(), dlg->getPort());

        unsigned int bus = dlg->getSrcpBus();
        if (bus != srcpBus) {
            srcpBus = bus;
            emit srcpBusChanged(srcpBus);
        }
        autoConnect = dlg->getAutoConnect();
        autoPowerOn = dlg->getAutoPowerOn();

        modified = true;

        if (autoConnect)
            slotDaemonConnect();
    }
    delete dlg;
}

void MainWindow::slotOptionsPreferences()
{
    PreferencesDialog *dlg = new PreferencesDialog(this);
	// TODO: fill parameters
    dlg->setDialogData(autoLoad, cAutoloadFile, mfxMaster, rregCntr, scale);

    if (dlg->exec() == QDialog::Accepted) {
        autoLoad = dlg->getAutoload();
        cAutoloadFile = dlg->getAutoloadFile();
		mfxMaster =	dlg->getMfxMaster();
		unsigned int tmprreg = dlg->getRregCntr();
		if (rregCntr != tmprreg) {
			rregCntr = tmprreg;
			handleRregRequest();
		}
        int tmpscale = dlg->getScale();
        if (scale != tmpscale) {
            scale = tmpscale;
            emit scaleChanged(scale);
        }
        savePreferences();
    }

    delete dlg;
}

void MainWindow::handleRregRequest()
{
//	SET <bus> SM 0 BIND <rregCntr>

	if (mfxMaster) {
	    //	qWarning("handleRregRequest");
	    ignoreAcks++;
		SmMessage sm = SmMessage(SrcpMessage::mtSmSet, srcpBus, 0,
            							SmMessage::smtBind, rregCntr);
    	emit sendSrcpMessage(sm);
    }
}

/*
 * load user preferences
 */
void MainWindow::loadPreferences()
{
    QString s, key, value;

    QFile file(QDir::homePath() + "/" + DTCLTINYRC);
    if (!file.open(QIODevice::ReadOnly)) {
        emit statusMessage(tr("Error: Could not open configuration "
                    "file: ~/%1").arg(DTCLTINYRC));
        return;
    }
    QTextStream ts(&file);
    SETUTF8;

    while (!ts.atEnd()) {
        s = ts.readLine();
        /* ignore comment lines */
        if (!s.startsWith("#")) {
            key = s.section(KS, 0, 0);
            value = s.section(KS, 1).trimmed();
            /* values are read sequence independent */
            if (key.compare(PF_AUTOLOAD) == 0)
                autoLoad = (bool) value.toInt();
            else if (key.compare(PF_AUTOLOADFILE) == 0)
                cAutoloadFile = value;
			else if (key.compare(PF_MFXMASTER) == 0)	   
			    mfxMaster = (bool) value.toInt();
			else if (key.compare(PF_RREGCOUNTER) == 0)   
                rregCntr = value.toUInt();
            else if (key.compare(PF_SCALE) == 0)
                scale = value.toInt();
            else if (key.compare(PF_LASTDIR) == 0)
                lastDir = value;
            else if (key.compare(PF_SHOWTOOLBAR) == 0)
                actionViewToolbar->setChecked(value.toInt());
            else if (key.compare(PF_SHOWSTATUSBAR) == 0)
                actionViewStatusline->setChecked(value.toInt());
            else if (key.compare(CF_RECENTFILE) == 0) {
                addRecentlyOpenedFile(value, recentFiles);
            }
        }
    }
    file.close();
}


void MainWindow::savePreferences()
{
    QFile file(QDir::homePath() + "/" + DTCLTINYRC);

    if (!file.open(QIODevice::WriteOnly)) {
        emit statusMessage(tr("Error: Could not save configuration "
                    "file: ~/%1").arg(QString(DTCLTINYRC)));
        return;
    }

    QDateTime dt = QDateTime::currentDateTime();
    QTextStream ts(&file);
    SETUTF8;

    ts << "# " PACKAGE "configuration file" << endl;
    ts << "# last modified: " << dt.toString(Qt::ISODate) << endl;
    ts << "#" << endl;
    ts << PF_AUTOLOAD << KS << autoLoad << endl;
    ts << PF_AUTOLOADFILE << KS << cAutoloadFile << endl;
    ts << PF_MFXMASTER << KS << mfxMaster << endl; 
	ts << PF_RREGCOUNTER << KS << rregCntr << endl; 
    ts << PF_SCALE << KS << scale << endl;
    ts << PF_LASTDIR << KS << lastDir << endl;
    ts << PF_SHOWTOOLBAR << KS
        << actionViewToolbar->isChecked() << endl;
    ts << PF_SHOWSTATUSBAR << KS
        << actionViewStatusline->isChecked() << endl;

    QStringList::Iterator it;
    for (int i = recentFiles.size() - 1; i >= 0; --i) {
        ts << CF_RECENTFILE << KS << recentFiles.at(i) << endl;
    }

    file.close();
}


void MainWindow::closeEvent(QCloseEvent* e)
{
    if (isSave())
        e->accept();
    else
        e->ignore();
}

/*
 * connect to SRCP server
 */
void MainWindow::slotDaemonConnect()
{
    if (!commandPort->hasServerConnection())
        commandPort->serverConnect();
}

/*
 * disconnect from SRCP server
 */
void MainWindow::slotDaemonDisconnect()
{
    commandPort->serverDisconnect();
    infoPort->serverDisconnect();
}

/*
 * reset SRCP server
 */
void MainWindow::slotDaemonReset()
{
    if (QMessageBox::Yes == QMessageBox::warning(this, tr("Server reset"),
            tr("You are going to reset the SRCP server.\nAre you sure?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
    	ignoreAcks = 1;
        ServerMessage sm = ServerMessage(SrcpMessage::mtServerReset);
        sendSrcpMessage(sm);
        emit statusMessage(tr("Daemon reset send"));
    }
}

/*
 * shutdown SRCP server
 */
void MainWindow::slotDaemonShutdown()
{
    if (QMessageBox::Yes == QMessageBox::warning(this,
                tr("Server shutdown"),
            tr("You are going to shutdown the SRCP server.\nAre you sure?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
    ServerMessage sm = ServerMessage(SrcpMessage::mtServerTerm);
    sendSrcpMessage(sm);
    emit statusMessage(tr("Daemon shutdown send"));
    updateDaemonMenuItems();
    }
}

/*
 * show dialog with server information
 */
void MainWindow::slotDaemonInfo()
{
    ServerInfoDialog* sid = new ServerInfoDialog(this);
    if (sid == NULL)
        return;

    sid->setCommandSessionData(
            commandPort->getSrcpServer(),
            commandPort->getSrcpVersion(),
            commandPort->getSrcpOther(),
            commandPort->getSessionId());

    sid->setInfoSessionData(
            infoPort->getSrcpServer(),
            infoPort->getSrcpVersion(),
            infoPort->getSrcpOther(),
            infoPort->getSessionId());

    sid->exec();
    delete sid;
}


void MainWindow::slotDaemonPower()
{
    if (powerOn) {
        PowerMessage pm = PowerMessage(SrcpMessage::mtPowerSet,
                 srcpBus, false);
        sendSrcpMessage(pm);
        updatePowerState(false);
    }
    else {
        PowerMessage pm = PowerMessage(SrcpMessage::mtPowerSet,
                 srcpBus, true);
        sendSrcpMessage(pm);
        updatePowerState(true);
    }

}

/*Initialize emergency halt for every controller and finally power down*/
void MainWindow::slotDaemonEmergency()
{
    if (controllers.size() > 0) {
        LocoControl * lc = controllers.first();
        lc->slotEmergencyHalt(true);
        state = csEmergency;
    }
}


// programmer with NMRA layout
void MainWindow::slotProgramNMRA()
{
    Programmer *dp = new Programmer(this, NMRA, powerOn, srcpBus);
    connect(dp, SIGNAL(sendSrcpMessage(const SrcpMessage&)),
            this, SLOT(sendSrcpMessage(const SrcpMessage&)));

    dp->exec();
    delete dp;
}


// programmer with Uhlenbrock layout
void MainWindow::slotProgramUhl()
{
    Programmer *dp = new Programmer(this, UHL, powerOn, srcpBus);
    connect(dp, SIGNAL(sendSrcpMessage(const SrcpMessage&)),
            this, SLOT(sendSrcpMessage(const SrcpMessage&)));

    dp->exec();
    delete dp;
}


void MainWindow::slotHelpHelp()
{
    QString sURL;
	QStringList translations;

    QLocale loc = QLocale::system();
    translations << "de";
    QString cl = loc.name();
    cl.resize(2);

    if (translations.contains(cl))
        sURL = QString("%1/index.%2.html").arg(HTML_DOC_DIR).arg(cl);
    else
        sURL = QString("%1/index.html").arg(HTML_DOC_DIR);

    QDesktopServices::openUrl(QUrl::fromLocalFile(sURL));
}


void MainWindow::slotHelpAbout()
{
    AboutDialog* ad = new AboutDialog(this);
    if (ad == NULL)
        return;

    ad->exec();
    delete ad;
}


void MainWindow::slotHelpAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}


void MainWindow::slotLocomotiveRemove(int cid)
{
    for (int i = 0; i < controllers.size(); ++i) {
        if (controllers.at(i)->hasId(cid)){
            LocoControl* lc = controllers.takeAt(i);
            lc->deleteLater();
            modified = true;
        }
    }
}


/*
 * send SRCP message to command port
 *
void MainWindow::sendSrcpMessageString(const QString& sm)
{
    commandPort->sendToServer(sm);
}*/


void MainWindow::sendSrcpMessage(const SrcpMessage& sm)
{
    commandPort->sendToServer(sm.getSrcpMessageStr());
}

/*
 * update all daemon related menu items
 */
void MainWindow::updateDaemonMenuItems()
{
    actionDaemonConnect->setEnabled(!serverConnected);
    actionDaemonDisconnect->setEnabled(serverConnected);
    actionDaemonReset->setEnabled(serverConnected);
    actionDaemonShutdown->setEnabled(serverConnected);
    actionDaemonInfo->setEnabled(serverConnected);
    actionDaemonPower->setEnabled(serverConnected);
    actionProgrammerNmra->setEnabled(serverConnected);
    actionProgrammerUhl->setEnabled(serverConnected);

    updatePowerMenuItems();
}

/*
 * update all layout power related menu items
 */
void MainWindow::updatePowerMenuItems()
{
    if (serverConnected) {
        actionDaemonPower->setEnabled(true);

        bool enableEmergency = powerOn && (controllers.count() > 0);
        actionDaemonEmergency->setEnabled(enableEmergency);
    }
    else {
        actionDaemonPower->setEnabled(false);
        actionDaemonEmergency->setEnabled(false);
    }

    if (powerOn)
        actionDaemonPower->setIcon(QPixmap(stoppower));
    else
        actionDaemonPower->setIcon(QPixmap(startpower));
}


void MainWindow::readAutoloadFile()
{
    if (!autoLoad)
        return;

    // if autoload file from config data does
    // not exist ask user to change options

    if (!QFile::exists(cAutoloadFile)) {
        qApp->beep();
        int choice = QMessageBox::warning(this, tr("Autoloader failed"),
                        tr("The selected autoload file '%1'\n"
                           "does not exist. Please adjust your"
                           " options.").arg(cAutoloadFile),
						QMessageBox::Ok | QMessageBox::Ignore);
        if (choice == QMessageBox::Ok)
            slotOptionsPreferences();
    }
    else
        openFile(cAutoloadFile);
}

/*
 * create new application window and open file
 * */
void MainWindow::openFileWindow(const QString& fn)
{
    MainWindow *sw = new MainWindow();
    sw->resize(640, 480);
    sw->show();
    sw->openFile(fn);
}


void MainWindow::setupRecentFilesMenu()
{
    fileRecentlyOpenedFiles->clear();

    if (recentFiles.count() > 0) {
        fileRecentlyOpenedFiles->setEnabled(true);
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            fileRecentlyOpenedFiles->addAction(*it);
        }
    }
    else
        fileRecentlyOpenedFiles->setEnabled(false);
}


void MainWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (isSave())
            openFile(action->text());
    }
}


void MainWindow::addRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    QString path = fi.absoluteFilePath();
    int index = lst.indexOf(path);

    // already first entry
    if (index == 0)
        return;

    // not first entry but found
    else if (index > 0)
        lst.removeAt(index);

    // not found and new entry necessary
    else  {
        if (lst.count() >= 6)
        lst.pop_back();
    }

    lst.prepend(path);
}

