// preferencesdlg.h - adapted for raidtcl project 2018 by Rainer Müller 

/***************************************************************************
                               preferencesdlg.h
                             -----------------
    begin                : 2004-10-26
    copyright            : (C) 2004-2007 by Guido Scholz
    email                : guido.scholz@bayernline.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREFERENCESDLG_H
#define PREFERENCESDLG_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

static const char DTCLFILEEXT[] = ".dtcl";


class PreferencesDialog: public QDialog {
    Q_OBJECT

        QLineEdit *fileLE, *rregLE;
        QCheckBox *autoloadCB, *mfxMasterCB;
        QPushButton *filePB;
        QComboBox *scaleCB;

    public:
        PreferencesDialog(QWidget* parent = 0);
        void setDialogData(bool autoload, const QString& filename,
						bool master, int regcount, int scale);
        bool getAutoload();
        QString getAutoloadFile();
		bool getMfxMaster();
		unsigned int getRregCntr();
        int getScale();

    protected slots:
        void chooseFile();
        void slotAutoloadChanged(bool);
        void slotMfxMasterChanged(bool);
};

#endif  //PREFERENCESDIALOG_H
