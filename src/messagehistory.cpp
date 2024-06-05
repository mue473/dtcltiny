// messagehistory.cpp - adapted for raidtcl project 2018 -2023 by Rainer Müller

/***************************************************************************
 messagehistory.cpp
 ------------------
 Begin      : 2007-08-20
 Copyright  : (C) 2007 by Guido Scholz
 E-Mail     : guido-scholz@gmx.net
 Description: Stacked set of Combo Boxex to collect different message types
              in separated lists.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QAction>
#include <QDateTime>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTextStream>

#include "messagehistory.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    #define endl Qt::endl
#endif

// max number of history lines
enum {
    MAX_HISTORY = 100
};



MessageHistory::MessageHistory(QWidget* parent): QWidget(parent),
    visibleLine(vlHint), showTime(true)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    lblStack = new QStackedWidget(this);
    lblStack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(lblStack);

    QLabel *HintLabel = new QLabel(tr("Hints"), lblStack);
    HintLabel->setIndent(3);
    lblStack->addWidget(HintLabel);

    QLabel *CmdLabel = new QLabel(tr("Commands"), lblStack);
    CmdLabel->setIndent(3);
    lblStack->addWidget(CmdLabel);

    QLabel *InfoLabel = new QLabel(tr("Infoport"), lblStack);
    InfoLabel->setIndent(3);
    lblStack->addWidget(InfoLabel);

    cbStack = new QStackedWidget(this);
    layout->addWidget(cbStack);
    cbStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // set mono space font for message texts
    QFont f = QFont("Courier", 10);

    hintCB = new QComboBox(cbStack);
    hintCB->setMaxVisibleItems(15);
    hintCB->setFont(f);
    cbStack->addWidget(hintCB);

    commandCB = new QComboBox(cbStack);
    commandCB->setMaxVisibleItems(15);
    commandCB->setFont(f);
    cbStack->addWidget(commandCB);

    infoCB = new QComboBox(cbStack);
    infoCB->setMaxVisibleItems(15);
    infoCB->setFont(f);
    cbStack->addWidget(infoCB);
}

/*
 * cyclic toggling between three history lines
 */
void MessageHistory::toggleLine()
{
    if (visibleLine == vlHint)
        visibleLine = vlCommand;
    else if (visibleLine == vlCommand)
        visibleLine = vlInfo;
    else if (visibleLine == vlInfo)
        visibleLine = vlHint;

    lblStack->setCurrentIndex(visibleLine);
    cbStack->setCurrentIndex(visibleLine);
}

/*
 * add a message to the hint combo box
 */
void MessageHistory::addHint(const QString& msg)
{
    if (hintCB->count() == MAX_HISTORY)
        hintCB->removeItem(0);

    if (showTime) {
        QString timemsg = getCurrentTime();
        timemsg.append(" ");
        timemsg.append(msg);
        hintCB->addItem(timemsg);
    }
    else
        hintCB->addItem(msg);
    hintCB->setCurrentIndex(hintCB->count() - 1);
}

/*
 * add a message to the command combo box
 */
void MessageHistory::addCommand(const QString& msg)
{
    if (commandCB->count() == MAX_HISTORY)
        commandCB->removeItem(0);

    if (showTime) {
        QString timemsg = getCurrentTime();
        timemsg.append(" ");
        timemsg.append(msg);
        commandCB->addItem(timemsg);
    }
    else
        commandCB->addItem(msg);
    commandCB->setCurrentIndex(commandCB->count() - 1);
}

/*
 * add a message to the info combo box
 */
void MessageHistory::addInfo(const QString& msg)
{
    if (infoCB->count() == MAX_HISTORY)
        infoCB->removeItem(0);

    if (showTime) {
        QString timemsg = getCurrentTime();
        timemsg.append(" ");
        timemsg.append(msg);
        infoCB->addItem(timemsg);
    }
    else
        infoCB->addItem(msg);
    infoCB->setCurrentIndex(infoCB->count() - 1);
}

/*
 * get a formated string with current time
 */
QString MessageHistory::getCurrentTime()
{
    QTime ctime = QTime::currentTime();
    QString timestr = ctime.toString("hh:mm:ss.zzz");
    return timestr;
}

void MessageHistory::enableTime(bool enable)
{
    showTime = enable;
}

bool MessageHistory::isTimeEnabled()
{
    return showTime;
}

void MessageHistory::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        QMenu menu;
        QAction* saveAction = new QAction(tr("&Save log..."), &menu);
        QAction* timeAction;
        if (showTime)
            timeAction = new QAction(tr("Hide &time"), &menu);
        else
            timeAction = new QAction(tr("Show &time"), &menu);
        menu.addAction(saveAction);
        menu.addAction(timeAction);

        QAction* menuAction = menu.exec(QCursor::pos());
        if (menuAction == saveAction)
            saveLog();
        else if (menuAction == timeAction)
            enableTime(!showTime);

        event->accept();
    }
    else if (event->button() == Qt::LeftButton) {
        toggleLine();
        event->accept();
    }
}

void MessageHistory::saveLog()
{
    QLabel* currentlabel = (QLabel*)lblStack->currentWidget();

    QString fn = QFileDialog::getSaveFileName(this, tr("Save log text"),
            QDir::homePath() + "/" + currentlabel->text() + ".txt",
            tr("Text files (*.txt)"), NULL, QFileDialog::DontUseNativeDialog);

    if (!fn.isEmpty()) {

        if (!fn.endsWith(".txt"))
            fn.append(".txt");

        QFile file(fn);
        if (file.open(QIODevice::WriteOnly) == false) {
            QMessageBox::warning(this, tr("Error"),
                    tr("Could not open %1 for writing.").
                    arg(fn));
            return;
        }

        QTextStream ts(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        ts.setEncoding(QStringConverter::Utf8);
#else
        ts.setCodec("utf-8");
#endif

        QComboBox* currentcb = (QComboBox*)cbStack->currentWidget();
        for (int i = 0; i < currentcb->count(); i++) {
            ts << currentcb->itemText(i) << endl;
        }
        file.close();
    }
}
