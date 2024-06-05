/***************************************************************************
    crcfmessage.h
    -----------------------------------
    copyright: (C) 2008 by Guido Scholz
    email    : guido-scholz@gmx.net
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
    This is the headerfile for crcfmessage.cpp
 ***************************************************************************/

#ifndef CRCFMESSAGE_H
#define CRCFMESSAGE_H

#include <QString>


class CrcfMessage
{
    public:
        enum CrcfActor {acNone = 0, acLayout, acRoute, acRwcc,
            acSection, acTrain};

        enum CrcfMethod {meNone = 0, meSet, meGet, meInfo};

        enum CrcfAttribute {atNone = 0, atId, atName, atState, atType,
            atTrain, atRows, atColumns, atMode, atTableLight, atLength,
            atSpeedLimit, atEntrySignalAspect, atExitSignalAspect,
            atDirection, atNextRoute, atSpeed};

        CrcfMessage(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute);
        CrcfMessage(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute, unsigned int);
        CrcfMessage(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute, const QString&);
        virtual ~CrcfMessage();

        CrcfMessage::CrcfActor getActor() const;
        CrcfMessage::CrcfMethod getMethod() const;
        CrcfMessage::CrcfAttribute getAttribute() const;
        QString getActorStr() const;
        QString getMethodStr() const;
        QString getAttributeStr() const;
        QString getAttValueStr() const;
        unsigned int getActorId() const;
        unsigned int getAttValue() const;
        QString getMessage() const;
        void setAttribute(CrcfAttribute);

        static QString message(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute);
        static QString message(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute, unsigned int);
        static QString message(CrcfActor, unsigned int, CrcfMethod,
                CrcfAttribute, const QString&);

        static CrcfMessage* parse(const QString&);

    private:
        static QString actorStr(CrcfActor);
        static QString methodStr(CrcfMethod);
        static QString attributeStr(CrcfAttribute);
        CrcfActor actor;
        unsigned int actor_id;
        CrcfMethod method;
        CrcfAttribute attribute;
        unsigned int attvalue;
        QString attvaluestr;
};
#endif

