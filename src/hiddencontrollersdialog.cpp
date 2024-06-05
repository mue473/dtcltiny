// hiddencontrollersdialog.cpp - adapted for raidtcl project 2018 - 2020 by Rainer Müller

/*
 * hiddencontrollersdialog.cpp
 * ---------------------------
 * Begin
 *   2016-01-10
 *
 * Copyright
 *   (C) 2016 Guido Scholz <guido-scholz@gmx.net>
 *
 * Description
 *   Dialog window to display hidden controllers
 *
 * License
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */


#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "hiddencontrollersdialog.h"


HiddenControllersDialog:: HiddenControllersDialog(QWidget* parent):
 QDialog(parent)
{
    setModal(false);
    setObjectName("HiddenControllersDialog");
    setWindowTitle(tr("Show all controlled items"));
    setMinimumSize(700, 300);

    QVBoxLayout* baseLayout = new QVBoxLayout();

	listWidget = new  QTreeWidget();
	listWidget->setIconSize(QSize(150, 50));
    listWidget->setColumnCount(6);
    listWidget->setColumnWidth(0, 160);

    QStringList labels;
    labels << ""  <<  tr("Name") <<  tr("Type") << tr("used") << tr("operation") << tr("Cntl");
    listWidget->setHeaderLabels(labels);

    listWidget->sortByColumn(1, Qt::AscendingOrder);
    listWidget->setSortingEnabled(true);

    // show button
    showButton = new QPushButton(tr("&Show"));
    showButton->setAutoDefault(false);
    connect(showButton, SIGNAL(pressed()),
            this, SLOT(slotShowSelectedController()));

    // OK button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

    buttonBox->addButton(showButton, QDialogButtonBox::ActionRole);

    baseLayout->addWidget(listWidget);
    baseLayout->addWidget(buttonBox);
    setLayout(baseLayout);
}


void HiddenControllersDialog::setControllersList(
    	const QList<LocoControl *>& hlist)
{

    for (int i = 0; i < hlist.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(QTreeWidgetItem::UserType);

        item->setIcon(0, hlist.at(i)->getLocoPixmap());
		item->setText(1, hlist.at(i)->getListText());
		item->setText(2, ((hlist.at(i)->isHidden()) ? "H" : " "));
		item->setText(3, hlist.at(i)->getLastUsed());
		item->setText(4, hlist.at(i)->getOperationTime());
		item->setText(5, QString("%1").arg(hlist.at(i)->getId(), 3));

        listWidget->addTopLevelItem(item);
    }
    showButton->setEnabled(listWidget->topLevelItemCount() > 0);
}


void HiddenControllersDialog::slotShowSelectedController()
{
    int cid;

    if (listWidget->topLevelItemCount() == 0) return;

    QTreeWidgetItem *item = listWidget->currentItem();
    if (item != NULL)  {
    	cid = item->text(5).toInt();
    	emit showController(cid);
    }
}
