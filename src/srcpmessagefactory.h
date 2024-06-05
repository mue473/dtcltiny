/***************************************************************************
 srcpmessagefactory.h
 ----------------
 Copyright    : 2012 by Guido Scholz
 E-Mail       : guido-scholz@gmx.net
 Begin        : 2012-01-08
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
    This is the headerfile for srcpmessagefactory.cpp
 ***************************************************************************/

#ifndef SRCPMESSAGEFACTORY_H
#define SRCPMESSAGEFACTORY_H

#include <QObject>
#include <QString>

#include "srcpmessage.h"


class SrcpMessageFactory: public QObject
{
    Q_OBJECT

    private:
        QString error;

    public:
        SrcpMessageFactory(QObject* parent = 0);
        //~SrcpMessageFactory();

        SrcpMessage* parse(const QString&);
        //virtual SrcpMessage* parse(QString&) = 0;
        QString getErrorStr() const;
};
#endif

