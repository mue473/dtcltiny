/***************************************************************************
 routeswindow.h
 -----------------
 Begin      : 2016-02-28
 Copyright  : (C) 2016 by Guido Scholz
 E-Mail     : guido-scholz@gmx.net
 Description: Window to display collected routes
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ROUTESWINDOW_H
#define ROUTESWINDOW_H

#include <QDockWidget>
#include <QTreeWidget>
#include <QPushButton>

#include "routesviewoptionsdialog.h"


enum SignalAspect {saNone = 0, saHp0, saHp1, saHp2, saSh1};

enum RouteRequest {requestNone = 0, requestName, requestLength,
    requestSpeedlimit, requestExitsignalaspect, requestEntrysignalaspect};

enum RouteState {rsNone = 0, rsUnused, rsUsed, rsActivated, rsReserved};

//container for data of used/known route
struct Route{
    unsigned int id;
    QString name;
    unsigned int length;
    unsigned int speedlimit;
    unsigned int entrysignalaspect;
    unsigned int exitsignalaspect;
    RouteState state;
    RouteRequest request;
};


class RoutesWindow: public QDockWidget {
    Q_OBJECT

    RoutesViewOptions rvo;
    QTreeWidget *treeWidget;
    QString getSignalAspect(unsigned int);
    void addRouteitem(const Route&);

public:
    RoutesWindow(QWidget* parent = 0, const QString& ln = "", const
            QString& la = "", const RoutesViewOptions& arvo = {true,
            true, true, true, true, false, 2});

    void setRouteList(const QList<Route>& rlist);
    void setRouteViewOptions(const RoutesViewOptions&);
    void clearRoutes();

public slots:
    void updateRouteData(const Route&);

signals:

};

#endif    //ROUTESWINDOW_H

