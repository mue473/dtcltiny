/***************************************************************************
    crcfmessage.cpp
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
    This code implements a class for CRCF messages which can be
    exchanged via SRCP Generic Messages (GM).
    It implements methods to parse a CRCF message string to binary message
    content and the other way around.
 ***************************************************************************/

#include <QStringList>
#include <QUrl>

#include "crcfmessage.h"

static const char CS_SET[] =              "SET";
static const char CS_GET[] =              "GET";
static const char CS_INFO[] =             "INFO";
static const char CS_COLUMNS[] =          "COLUMNS";
static const char CS_DIRECTION[] =        "DIRECTION";
static const char CS_ENTRYSIGNALASPECT[] = "ENTRYSIGNALASPECT";
static const char CS_EXITSIGNALASPECT[] =  "EXITSIGNALASPECT";
static const char CS_ID[] =               "ID";
static const char CS_LAYOUT[] =           "LAYOUT";
static const char CS_LENGTH[] =           "LENGTH";
static const char CS_MODE[] =             "MODE";
static const char CS_NAME[] =             "NAME";
static const char CS_NEXTROUTE[] =        "NEXTROUTE";
static const char CS_ROUTE[] =            "ROUTE";
static const char CS_ROWS[] =             "ROWS";
static const char CS_RWCC[] =             "RWCC";
static const char CS_SECTION[] =          "SECTION";
static const char CS_SPEED[] =            "SPEED";
static const char CS_SPEEDLIMIT[] =       "SPEEDLIMIT";
static const char CS_STATE[] =            "STATE";
static const char CS_TABLELIGHT[] =       "TABLELIGHT";
static const char CS_TRAIN[] =            "TRAIN";
static const char CS_TYPE[] =             "TYPE";


// <actor> <actorid> <method> <attribute>
//   ROUTE     11     GET      TRAIN
/*constructor without attribute (GET)*/
CrcfMessage::CrcfMessage(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat):
    actor(cac), actor_id(aid), method(cme), attribute(cat),
    attvalue(0), attvaluestr("")
{
}

// <actor> <actorid> <method> <attribute> <attribute-value>
//   ROUTE     11     GET      TRAIN
/*constructor with integer attribute value*/
CrcfMessage::CrcfMessage(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat, unsigned int value):
    actor(cac), actor_id(aid), method(cme), attribute(cat),
    attvalue(value), attvaluestr("")
{
}


/*constructor with string attribute value*/
CrcfMessage::CrcfMessage(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat, const QString& valuestr):
    actor(cac), actor_id(aid), method(cme), attribute(cat), attvalue(0)
{
    QUrl url = QUrl(valuestr);
    attvaluestr = url.toString();
}


// just to avoid compiler warnings
CrcfMessage::~CrcfMessage()
{
}


CrcfMessage::CrcfActor CrcfMessage::getActor() const
{
    return actor;
}


QString CrcfMessage::getActorStr() const
{
    return actorStr(actor);
}


QString CrcfMessage::actorStr(CrcfActor ac)
{
    QString acStr = "";

    switch (ac) {
        case acRoute:
            acStr = CS_ROUTE;
            break;
        case acSection:
            acStr = CS_SECTION;
            break;
        case acTrain:
            acStr = CS_TRAIN;
            break;
        case acLayout:
            acStr = CS_LAYOUT;
            break;
        case acRwcc:
            acStr = CS_RWCC;
            break;
        default:
            break;
    }
    return acStr;
}


unsigned int CrcfMessage::getActorId() const
{
    return actor_id;
}


CrcfMessage::CrcfMethod CrcfMessage::getMethod() const
{
    return method;
}


QString CrcfMessage::getMethodStr() const
{
    return methodStr(method);
}


QString CrcfMessage::methodStr(CrcfMethod me)
{
    QString meStr = "";

    switch (me) {
        case meInfo:
            meStr = CS_INFO;
            break;
        case meSet:
            meStr = CS_SET;
            break;
        case meGet:
            meStr = CS_GET;
            break;
        default:
            break;
    }
    return meStr;
}


CrcfMessage::CrcfAttribute CrcfMessage::getAttribute() const
{
    return attribute;
}


QString CrcfMessage::getAttributeStr() const
{
    return attributeStr(attribute);
}


QString CrcfMessage::attributeStr(CrcfAttribute at)
{
    QString atStr = "";

    switch (at) {
        case atId:
            atStr = CS_ID;
            break;
        case atName:
            atStr = CS_NAME;
            break;
        case atState:
            atStr = CS_STATE;
            break;
        case atType:
            atStr = CS_TYPE;
            break;
        case atTrain:
            atStr = CS_TRAIN;
            break;
        case atRows:
            atStr = CS_ROWS;
            break;
        case atColumns:
            atStr = CS_COLUMNS;
            break;
        case atMode:
            atStr = CS_MODE;
            break;
        case atTableLight:
            atStr = CS_TABLELIGHT;
            break;
        case atLength:
            atStr = CS_LENGTH;
            break;
        case atSpeedLimit:
            atStr = CS_SPEEDLIMIT;
            break;
        case atEntrySignalAspect:
            atStr = CS_ENTRYSIGNALASPECT;
            break;
        case atExitSignalAspect:
            atStr = CS_EXITSIGNALASPECT;
            break;
        case atDirection:
            atStr = CS_DIRECTION;
            break;
        case atNextRoute:
            atStr = CS_NEXTROUTE;
            break;
        case atSpeed:
            atStr = CS_SPEED;
            break;
        default:
            break;
    }
    return atStr;
}


unsigned int CrcfMessage::getAttValue() const
{
    return attvalue;
}


/* return value string; if string contains white spaces escape it using
 * the character '"'*/
QString CrcfMessage::getAttValueStr() const
{
    return attvaluestr;
}

/*
 * Parse CRCF message and return pointer to new message instance, if
 * message was valid.
 * <actor> <actor_id> <method> <attribute> [<value>]
 *    1       2          3         4          5
 */
CrcfMessage* CrcfMessage::parse(const QString& msg)
{
    CrcfActor actor;
    CrcfMethod cm;
    CrcfAttribute cat;
    unsigned int aid = 0;
    unsigned int value = 0;

    QStringList tokens = msg.simplified().split(' ');
    if (tokens.count() < 4)
        // error message to short
        return NULL;

    // token 1: actor
    if (CS_ROUTE == tokens[0]) {
        actor = acRoute;
    }
    else if (CS_LAYOUT == tokens[0]) {
        actor = acLayout;
    }
    else if (CS_SECTION == tokens[0]) {
        actor = acSection;
    }
    else if (CS_TRAIN == tokens[0]) {
        actor = acTrain;
    }
    else if (CS_RWCC == tokens[0]) {
        actor = acRwcc;
    }
    else {
        //error unsupported CRCF actor
        return NULL;
    }

    // token 2: actor id
    aid = tokens[1].toUInt();

    // token 3: method
    if (CS_SET == tokens[2]) {
        cm = meSet;
    }
    else if (CS_GET == tokens[2]) {
        cm = meGet;
    }
    else if (CS_INFO == tokens[2]) {
        cm = meInfo;
    }
    else {
        //error unsupported CRCF method
        return NULL;
    }

    // token 4: attribute (no context analysis)
    if (CS_STATE == tokens[3]) {
        cat = atState;
    }
    else if (CS_ID == tokens[3]) {
        cat = atId;
    }
    else if (CS_NAME == tokens[3]) {
        cat = atName;
    }
    else if (CS_TYPE == tokens[3]) {
        cat = atType;
    }
    else if (CS_TRAIN == tokens[3]) {
        cat = atTrain;
    }
    else if (CS_ROWS == tokens[3]) {
        cat = atRows;
    }
    else if (CS_COLUMNS == tokens[3]) {
        cat = atColumns;
    }
    else if (CS_MODE == tokens[3]) {
        cat = atMode;
    }
    else if (CS_TABLELIGHT == tokens[3]) {
        cat = atTableLight;
    }
    else if (CS_LENGTH == tokens[3]) {
        cat = atLength;
    }
    else if (CS_SPEEDLIMIT == tokens[3]) {
        cat = atSpeedLimit;
    }
    else if (CS_ENTRYSIGNALASPECT == tokens[3]) {
        cat = atEntrySignalAspect;
    }
    else if (CS_EXITSIGNALASPECT == tokens[3]) {
        cat = atExitSignalAspect;
    }
    else if (CS_DIRECTION == tokens[3]) {
        cat = atDirection;
    }
    else if (CS_NEXTROUTE == tokens[3]) {
        cat = atNextRoute;
    }
    else if (CS_SPEED == tokens[3]) {
        cat = atSpeed;
    }
    else {
        // Error: Unsupported CRCF attribute
        return NULL;
    }

    // token 5: attribute value
    if (meSet == cm || meInfo == cm) {
        if (tokens.count() == 5) {
            if (cat == atName)
                return new CrcfMessage(actor, aid, cm, cat, tokens[4]);
            value = tokens[4].toUInt();
        }
        else {
            //error unsupported CRCF operation
            return NULL;
        }
    }

    return new CrcfMessage(actor, aid, cm, cat, value);
}

/*assemble CRCF message string*/
QString CrcfMessage::getMessage() const
{
    if (atName != attribute) {
        if (meGet == method)
            return message(actor, actor_id, method, attribute);
        else
            return message(actor, actor_id, method, attribute, attvalue);
    }
    return message(actor, actor_id, method, attribute, attvaluestr);
}


void CrcfMessage::setAttribute(CrcfAttribute cat)
{
    attribute = cat;
}

/*static assemble CRCF message string, integer attribute value*/
//GET
QString CrcfMessage::message(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat)
{
    QString result = QString("%1 %2 %3 %4").arg(actorStr(cac)).arg(aid)
            .arg(methodStr(cme)).arg(attributeStr(cat));

    return result;
}

//SET and INFO
QString CrcfMessage::message(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat, unsigned int value)
{
    QString result = QString("%1 %2 %3 %4 %5").arg(actorStr(cac)).arg(aid)
            .arg(methodStr(cme)).arg(attributeStr(cat)).arg(value);

    return result;
}

/* Static assemble CRCF message string, string attribute value.
 * The value string gets URL encoded.*/
QString CrcfMessage::message(CrcfActor cac, unsigned int aid, CrcfMethod cme,
        CrcfAttribute cat, const QString& valuestr)
{
    QString result;

    if (meGet == cme)
        result = QString("%1 %2 %3 %4").arg(actorStr(cac)).arg(aid)
            .arg(methodStr(cme)).arg(attributeStr(cat));
    else {
        QUrl url = QUrl(valuestr);
        QString crcfurl = url.toEncoded();
        result = QString("%1 %2 %3 %4 %5").arg(actorStr(cac)).arg(aid)
            .arg(methodStr(cme)).arg(attributeStr(cat)).arg(crcfurl);
    }

    return result;
}

