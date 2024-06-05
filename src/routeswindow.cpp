/*
 * routewindow.cpp
 * ---------------------------
 * Begin
 *   2016-02-28
 *
 * Copyright
 *   (C) 2016 Guido Scholz <guido-scholz@gmx.net>
 *
 * Description
 *   Dialog window to display collected route data
 *
 * License
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include <QTreeWidgetItem>
#include <QHeaderView>

#include "routeswindow.h"

#include "pixmaps/route_activated.xpm"
#include "pixmaps/route_locked.xpm"
#include "pixmaps/route_none.xpm"
#include "pixmaps/route_unlocked.xpm"
#include "pixmaps/route_reserved.xpm"
#include "pixmaps/signal_hp0.xpm"
#include "pixmaps/signal_hp1.xpm"
#include "pixmaps/signal_hp2.xpm"
#include "pixmaps/signal_sh1.xpm"
#include "pixmaps/signal_hpx.xpm"
#include "pixmaps/signal_hpy.xpm"


enum RoutesViewColumns {rvcId = 0, rvcName, rvcEntryAspect, rvcExitAspect,
    rvcLength, rvcSpeed};

RoutesWindow:: RoutesWindow(QWidget* parent, const QString& ln,
        const QString& la, const RoutesViewOptions& arvo):
    QDockWidget(tr("%1 (%2) CRCF route data").arg(ln).arg(la), parent),
    rvo(arvo)
{
    setObjectName("RoutesWindow");
    setFeatures(DockWidgetClosable | DockWidgetMovable |
            DockWidgetFloatable);

    treeWidget =  new QTreeWidget();
    treeWidget->setUniformRowHeights(true);
    treeWidget->setAllColumnsShowFocus(true);
    treeWidget->setRootIsDecorated(false);
    treeWidget->setSelectionMode(QAbstractItemView::NoSelection);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem(
            QStringList() << tr("Id") << tr("Name") << tr("En Aspect") << tr("Ex Aspect")
            << tr("Length") << tr("Speed"), QTreeWidgetItem::UserType + 1);
    headerItem->setTextAlignment(rvcId, Qt::AlignRight);
    headerItem->setTextAlignment(rvcLength, Qt::AlignRight);
    headerItem->setTextAlignment(rvcSpeed, Qt::AlignRight);
    treeWidget->setHeaderItem(headerItem);
    treeWidget->header()->resizeSection(rvcId, 50);
    treeWidget->header()->resizeSection(rvcName, 80);
    treeWidget->header()->resizeSection(rvcEntryAspect, 80);
    treeWidget->header()->resizeSection(rvcExitAspect, 80);
    treeWidget->header()->resizeSection(rvcLength, 60);
    treeWidget->header()->resizeSection(rvcSpeed, 50);
    treeWidget->header()->setStretchLastSection(false);

    setRouteViewOptions(rvo);
    setWidget(treeWidget);
}

/*Fill list with routes*/
void RoutesWindow::setRouteList(const QList<Route>& rlist)
{
    for (int i = 0; i < rlist.size(); ++i)
        addRouteitem(rlist.at(i));
}

/*Update one single route*/
void RoutesWindow::updateRouteData(const Route& rt)
{
    bool found = false;

    QTreeWidgetItem* rootitem = treeWidget->invisibleRootItem();
    if (NULL == rootitem)
        return;

    for (int i = 0; i < rootitem->childCount(); ++i) {
        QTreeWidgetItem* item = rootitem->child(i);

        if (NULL == item)
            continue;

        if (rt.id == item->data(rvcId, Qt::UserRole).toUInt()) {
            switch (rt.state) {
                case rsUnused:
                    item->setIcon(rvcId, QPixmap(route_unlocked_xpm));
                    break;
                case rsUsed:
                    item->setIcon(rvcId, QPixmap(route_locked_xpm));
                    break;
                case rsActivated:
                    item->setIcon(rvcId, QPixmap(route_activated_xpm));
                    break;
                case rsReserved:
                    item->setIcon(rvcId, QPixmap(route_reserved_xpm));
                    break;
                default:
                    item->setIcon(rvcId, QPixmap(route_none_xpm));
                    break;
            }
            item->setText(rvcName, rt.name);
            item->setText(rvcEntryAspect, getSignalAspect(rt.entrysignalaspect));
            item->setText(rvcExitAspect, getSignalAspect(rt.exitsignalaspect));
            item->setText(rvcLength, QString::number(rt.length));
            item->setText(rvcSpeed, QString::number(rt.speedlimit));

            switch (rt.entrysignalaspect) {
                case saNone:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_hpx_xpm));
                    break;
                case saHp0:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_hp0_xpm));
                    break;
                case saHp1:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_hp1_xpm));
                    break;
                case saHp2:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_hp2_xpm));
                    break;
                case saSh1:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_sh1_xpm));
                    break;
                default:
                    item->setIcon(rvcEntryAspect, QPixmap(signal_hpy_xpm));
                    break;
            }

            switch (rt.exitsignalaspect) {
                case saNone:
                    item->setIcon(rvcExitAspect, QPixmap(signal_hpx_xpm));
                    break;
                case saHp0:
                    item->setIcon(rvcExitAspect, QPixmap(signal_hp0_xpm));
                    break;
                case saHp1:
                    item->setIcon(rvcExitAspect, QPixmap(signal_hp1_xpm));
                    break;
                case saHp2:
                    item->setIcon(rvcExitAspect, QPixmap(signal_hp2_xpm));
                    break;
                case saSh1:
                    item->setIcon(rvcExitAspect, QPixmap(signal_sh1_xpm));
                    break;
                default:
                    item->setIcon(rvcExitAspect, QPixmap(signal_hpy_xpm));
                    break;
            }

            found = true;
            break;
        }

    }
    if (!found)
        addRouteitem(rt);
}

/*Add new item to route list*/
void RoutesWindow::addRouteitem(const Route& rt)
{
    QStringList list;

    list << QString::number(rt.id)
        << rt.name
        << getSignalAspect(rt.entrysignalaspect)
        << getSignalAspect(rt.exitsignalaspect)
        << QString::number(rt.length)
        << QString::number(rt.speedlimit);

    QTreeWidgetItem* item = new
        QTreeWidgetItem((QTreeWidget*)NULL, list);

    switch (rt.state) {
        case rsUnused:
            item->setIcon(rvcId, QPixmap(route_unlocked_xpm));
            break;
        case rsUsed:
            item->setIcon(rvcId, QPixmap(route_locked_xpm));
            break;
        case rsActivated:
            item->setIcon(rvcId, QPixmap(route_activated_xpm));
            break;
        case rsReserved:
            item->setIcon(rvcId, QPixmap(route_reserved_xpm));
            break;
        default:
            item->setIcon(rvcId, QPixmap(route_none_xpm));
            break;
    }

    switch (rt.entrysignalaspect) {
        case saNone:
            item->setIcon(rvcEntryAspect, QPixmap(signal_hpx_xpm));
            break;
        case saHp0:
            item->setIcon(rvcEntryAspect, QPixmap(signal_hp0_xpm));
            break;
        case saHp1:
            item->setIcon(rvcEntryAspect, QPixmap(signal_hp1_xpm));
            break;
        case saHp2:
            item->setIcon(rvcEntryAspect, QPixmap(signal_hp2_xpm));
            break;
        case saSh1:
            item->setIcon(rvcEntryAspect, QPixmap(signal_sh1_xpm));
            break;
        default:
            item->setIcon(rvcEntryAspect, QPixmap(signal_hpy_xpm));
            break;
    }

    switch (rt.exitsignalaspect) {
        case saNone:
            item->setIcon(rvcExitAspect, QPixmap(signal_hpx_xpm));
            break;
        case saHp0:
            item->setIcon(rvcExitAspect, QPixmap(signal_hp0_xpm));
            break;
        case saHp1:
            item->setIcon(rvcExitAspect, QPixmap(signal_hp1_xpm));
            break;
        case saHp2:
            item->setIcon(rvcExitAspect, QPixmap(signal_hp2_xpm));
            break;
        case saSh1:
            item->setIcon(rvcExitAspect, QPixmap(signal_sh1_xpm));
            break;
        default:
            item->setIcon(rvcExitAspect, QPixmap(signal_hpy_xpm));
            break;
    }

    item->setTextAlignment(rvcId, Qt::AlignRight);
    item->setTextAlignment(rvcLength, Qt::AlignRight);
    item->setTextAlignment(rvcSpeed, Qt::AlignRight);

    item->setData(rvcId, Qt::UserRole, rt.id);
    treeWidget->addTopLevelItem(item);
    if (rvo.limitroutes) {
        while (treeWidget->topLevelItemCount() > (int)rvo.routelimit)
            treeWidget->takeTopLevelItem(0);
    }
}

/*Get signal aspect string*/
QString RoutesWindow::getSignalAspect(unsigned int sa)
{
    QString aspect;

    switch (sa) {
        case saNone:
            aspect = '-';
            break;
        case saHp0:
            aspect = "Hp0";
            break;
        case saHp1:
            aspect = "Hp1";
            break;
        case saHp2:
            aspect = "Hp2";
            break;
        case saSh1:
            aspect = "Sh1";
            break;
        default:
            aspect = '?';
            break;
    }

    return aspect;
}


/*Remove all route list items*/
void RoutesWindow::clearRoutes()
{
    treeWidget->clear();
}


void RoutesWindow::setRouteViewOptions(const RoutesViewOptions& arvo)
{
    rvo = arvo;
    if (rvo.showname)
        treeWidget->showColumn(rvcName);
    else
        treeWidget->hideColumn(rvcName);

    if (rvo.showentryaspect)
        treeWidget->showColumn(rvcEntryAspect);
    else
        treeWidget->hideColumn(rvcEntryAspect);

    if (rvo.showexitaspect)
        treeWidget->showColumn(rvcExitAspect);
    else
        treeWidget->hideColumn(rvcExitAspect);

    if (rvo.showlength)
        treeWidget->showColumn(rvcLength);
    else
        treeWidget->hideColumn(rvcLength);

    if (rvo.showspeed)
        treeWidget->showColumn(rvcSpeed);
    else
        treeWidget->hideColumn(rvcSpeed);

    //adjust route list to limit
    if (rvo.limitroutes) {
        while (treeWidget->topLevelItemCount() > (int)rvo.routelimit)
            treeWidget->takeTopLevelItem(0);
    }
}
