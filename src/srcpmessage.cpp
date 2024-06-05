// srcpmessage.cpp - adapted for raidtcl project 2018 by Rainer Müller 

/***************************************************************************
 srcpmessage.cpp
 ----------------
 Copyright    : 2008-2016 by Guido Scholz
 E-Mail       : guido-scholz@gmx.net
 Begin        : 2008-12-26
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
    This code implements classes for SRCP messages which can be
    exchanged between a SRCP-data containers the SRCP command sender
    class. It implements methods to translate binary data to SRCP-
    protocol dependend command strings.
 ***************************************************************************/

#include "srcpmessage.h"


/*
 * base class for all SRCP messages
 */

SrcpMessage::SrcpMessage(Type atype, unsigned int abus):
    type(atype), code(code0), bus(abus)
{
}


SrcpMessage::SrcpMessage(Type atype, Code acode,
        unsigned int abus): type(atype), code(acode), bus(abus)
{
}


SrcpMessage::~SrcpMessage()
{
}


void SrcpMessage::setCode(SrcpMessage::Code acode)
{
    code = acode;
}


SrcpMessage::Code SrcpMessage::getCode() const
{
    return code;
}


void SrcpMessage::setType(SrcpMessage::Type atype)
{
    type = atype;
}


SrcpMessage::Type SrcpMessage::getType() const
{
    return type;
}


void SrcpMessage::setBus(unsigned int abus)
{
    bus = abus;
}


unsigned int SrcpMessage::getBus() const
{
    return bus;
}


/*
 * class for acknowledge messages
 * Type may be: mtAckOk, mtAckError
 */

/* mtAckOk:
 *   <time> <code> OK
 *     0      1    2
 */
AckMessage::AckMessage(Type type, Code acode):
    SrcpMessage(type, acode, 0), sessionid(0), text("")
{
}


/* mtAckOk:
 *   <time> <code> OK GO <sessionid>
 *     0      1    2  3       4
 */
AckMessage::AckMessage(Type type, Code acode, unsigned int sid):
    SrcpMessage(type, acode, 0), sessionid(sid), text("")
{
}


/* mtAckOk:
 *   <time> <code> OK CONNECTIONMODE
 *     0      1    2        3
 */
/* mtAckError:
 *   <time> <code> ERROR  <error text>
 *     0      1      2          3
 */
AckMessage::AckMessage(Type type, Code acode, const QString& txt):
    SrcpMessage(type, acode, 0), sessionid(0), text(txt)
{
}


unsigned int AckMessage::getSessionId()
{
    return sessionid;
}


QString AckMessage::getText() const
{
    return text;
}

QString AckMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";
    return cmdStr;
}

/*
 * class for DESCRIPTION messages
 * Type may be: mtDescriptionGet, mtDescriptionInfo
 */

/* mtDescriptionGet:
 *   GET <bus> DESCRIPTION
 *    0    1        2
 */
DescriptionMessage::DescriptionMessage(Type type, unsigned int abus):
    SrcpMessage(type, abus)
{
}


/* mtDescriptionInfo:
 *   INFO <bus> DESCRIPTION <devicelist>
 *    0    1        2            3...
 */
DescriptionMessage::DescriptionMessage(Type type, Code acode,
        unsigned int abus, int adevicemap):
    SrcpMessage(type, acode, abus), devicemap(adevicemap)
{
}


/*
 * Create SRCP 0.8 style messages
 */
QString DescriptionMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtDescriptionGet:
            cmdStr = QString("GET %1 DESCRIPTION").arg(bus);
            break;
        case mtDescriptionInfo:
            if (code == code100)
                cmdStr = QString("100 INFO %1 DESCRIPTION%2")
                    .arg(bus).arg(getDevicelistStr());
            break;
        default:
            break;
    }

    return cmdStr;
}

QString DescriptionMessage::getDevicelistStr() const
{
    QString devlist = "";

    if ((devicemap & SrcpMessage::devFb) != 0)
        devlist += QString(" FB");

    if ((devicemap & SrcpMessage::devGa) != 0)
        devlist += QString(" GA");

    if ((devicemap & SrcpMessage::devGl) != 0)
        devlist += QString(" GL");

    if ((devicemap & SrcpMessage::devGm) != 0)
        devlist += QString(" GM");

    if ((devicemap & SrcpMessage::devLock) != 0)
        devlist += QString(" LOCK");

    if ((devicemap & SrcpMessage::devPower) != 0)
        devlist += QString(" POWER");

    if ((devicemap & SrcpMessage::devServer) != 0)
        devlist += QString(" SERVER");

    if ((devicemap & SrcpMessage::devSession) != 0)
        devlist += QString(" SESSION");

    if ((devicemap & SrcpMessage::devSm) != 0)
        devlist += QString(" SM");

    if ((devicemap & SrcpMessage::devTime) != 0)
        devlist += QString(" TIME");

    return devlist;
}


bool DescriptionMessage::hasGa() const
{
    return (devicemap & SrcpMessage::devGa) != 0;
}


bool DescriptionMessage::hasGl() const
{
    return (devicemap & SrcpMessage::devGl) != 0;
}


bool DescriptionMessage::hasGm() const
{
    return (devicemap & SrcpMessage::devGm) != 0;
}


bool DescriptionMessage::hasPower() const
{
    return (devicemap & SrcpMessage::devPower) != 0;
}
/*
 * class for FB messages
 * Type may be: mtFBInit, mtFbSet, mtFbGet, mtFbTerm, mtFbInfo
 */
/* mtFbGet:
 *   GET <bus> FB <addr>
 *    0    1   2    3
 */
FbMessage::FbMessage(Type type, unsigned int abus, unsigned int anaddress):
    SrcpMessage(type, abus), address(anaddress)
{
}

/*
 * mtFbSet:
 *   SET <bus> FB <addr> <value>
 *    0    1   2    3      4
 */
FbMessage::FbMessage(Type type, unsigned int abus,
        unsigned int addr, unsigned int avalue):
    SrcpMessage(type, abus), address(addr), value(avalue)
{
}

/* mtFbInfo:
 *   <time> 101 INFO <bus> FB
 *     0     1   2     3   4
 *   <time> 102 INFO <bus> FB
 *     0     1   2     3   4
 */
FbMessage::FbMessage(Type type, Code acode, unsigned int abus):
    SrcpMessage(type, acode, abus)
{
}

/* mtFbInfo:
 *   <time> 100 INFO <bus> FB <addr> <value>
 *     0     1   2     3   4    5       6
 */
FbMessage::FbMessage(Type type, Code acode, unsigned int abus,
        unsigned int addr, unsigned int avalue):
    SrcpMessage(type, acode, abus), address(addr), value(avalue)
{
}

/* mtFbInfo:
 *   <time> 103 INFO <bus> FB <addr> 1 GL <addr> <drivemode> <V>
 *     0     1   2     3   4    5    6 7    8         9       10
 */
FbMessage::FbMessage(Type type, Code acode, unsigned int abus,
        unsigned int addr, unsigned int avalue, Device abididev,
        unsigned int gladdr, unsigned int gldm, unsigned int glv):
    SrcpMessage(type, acode, abus), address(addr), value(avalue),
    bididev(abididev), gladdress(gladdr), gldrivemode(gldm),
    glvelocity(glv)
{
}

/*
 * Create SRCP 0.8 style messages
 */
QString FbMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtFbSet:
            cmdStr = QString("SET %1 FB %2 %3")
                .arg(bus).arg(address).arg(value);
            break;
        case mtFbGet:
            cmdStr = QString("GET %1 FB %2")
                .arg(bus).arg(address);
            break;
        case mtFbInfo:
            if (code == code100)
                cmdStr = QString("100 INFO %1 FB %2 %3")
                    .arg(bus).arg(address).arg(value);
            break;
        case mtFbInit:
            cmdStr = QString("INIT %1 FB");
            break;
        case mtFbTerm:
            cmdStr = QString("TERM %1 FB %2")
                .arg(bus).arg(address);
            break;
        default:
            break;
    }

    return cmdStr;
}


unsigned int FbMessage::getAddress() const
{
    return address;
}


unsigned int FbMessage::getGlAddress() const
{
    return gladdress;
}


unsigned int FbMessage::getValue() const
{
    return value;
}
/*
 * class for GA messages
 * Type may be: mtGaInit, mtGaSet, mtGaGet, mtGaTerm, mtGaInfo
 */
/* mtGaInit:
 *   INIT <bus> GA <addr> <prot>
 *    0     1   2    3      4
 */
GaMessage::GaMessage(Type type, unsigned int abus,
        unsigned int anaddress, Protocol pro):
    SrcpMessage(type, abus), address(anaddress), protocol(pro)
{
}

/* mtGaTerm:
 *   TERM <bus> GA <addr>
 *    0     1   2    3
 */
GaMessage::GaMessage(Type type, unsigned int abus,
        unsigned int anaddress):
    SrcpMessage(type, abus), address(anaddress), protocol(proNone)
{
}

/* mtGaGet:
 *   GET <bus> GA <addr> <port>
 *    0    1   2    3      4
 */
GaMessage::GaMessage(Type type, unsigned int abus,
        unsigned int anaddress, Protocol aprot, unsigned int aport):
    SrcpMessage(type, code100, abus), address(anaddress),
    protocol(aprot), port(aport)
{
}


/* mtGaSet:
 *   SET <bus> GA <addr> <port> <value> <delay>
 *    0    1   2    3      4       5       6
 */
GaMessage::GaMessage(Type type, unsigned int abus, unsigned int anaddress,
        Protocol aprot, unsigned int aport, unsigned int avalue,
        unsigned int adelay):
    SrcpMessage(type, code100, abus), address(anaddress),
    protocol(aprot), port(aport), value(avalue), delay(adelay)
{
}


/* mtGaInfo:
 * <time> 100 INFO <bus> GA <addr> <port> <value>
 *    0    1   2     3   4    5      6       7
 */
GaMessage::GaMessage(Type type, Code acode, unsigned int abus,
        unsigned int anaddress, Protocol aprot, unsigned int aport,
        unsigned int avalue):
    SrcpMessage(type, acode, abus), address(anaddress), protocol(aprot),
    port(aport), value(avalue)
{
}


/* mtGaInfo:
 * <time> 101 INFO <bus> GA <addr> <prot> <optionales>
 *    0    1   2     3   4    5      6
 */
GaMessage::GaMessage(Type type, Code acode, unsigned int abus,
        unsigned int anaddress, Protocol pro):
    SrcpMessage(type, acode, abus), address(anaddress), protocol(pro)
{
}


/* mtGaInfo:
 * <time> 102 INFO <bus> GA <addr>
 *    0    1   2     3   4    5
 */
GaMessage::GaMessage(Type type, Code acode, unsigned int abus,
        unsigned int anaddress):
    SrcpMessage(type, acode, abus), address(anaddress)
{
}


QString GaMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtGaSet:
            cmdStr = QString("SET %1 GA %2 %3 %4 %5")
                .arg(bus).arg(address).arg(port)
                .arg(value).arg(delay);
            break;
        case mtGaGet:
            cmdStr = QString("GET %1 GA %2 %3")
                .arg(bus).arg(address).arg(port);
            break;
        case mtGaInfo:
            switch (code) {
                case code100:
                    cmdStr = QString("100 INFO %1 GA %2 %3 %4")
                        .arg(bus).arg(address).arg(port).arg(value);
                    break;
                case code101:
                    cmdStr = QString("101 INFO %1 GA %2 %3")
                        .arg(bus).arg(address)
                        .arg(getProtocolStr(protocol));
                    break;
                case code102:
                    cmdStr = QString("102 INFO %1 GA %2")
                        .arg(bus).arg(address);
                    break;
                default:
                    break;
            }
            break;
        case mtGaInit:
            cmdStr = QString("INIT %1 GA %2 %3")
                .arg(bus).arg(address)
                .arg(getProtocolStr(protocol));
            break;
        case mtGaTerm:
            cmdStr = QString("TERM %1 GA %2")
                .arg(bus).arg(address);
            break;
        default:
            break;
    }

    return cmdStr;
}

/*return GA protocol string values*/
QString GaMessage::getProtocolStr(GaMessage::Protocol pro) const
{
    QString proStr = "P";

    switch (pro) {
        case proMM:
            proStr = "M";
            break;
        case proDCC:
            proStr = "N";
            break;
        case proServer:
    //        proStr = "P";
            break;
        case proSelectrix:
            proStr = "S";
            break;
        default:
            break;
    }
    return proStr;
}


GaMessage::Protocol GaMessage::getProtocolId(const QString& pro)
{
    if (pro == "M")
        return proMM;
    else if (pro == "N")
        return proDCC;
    else if (pro == "P")
        return proServer;
    else if (pro == "S")
        return proSelectrix;
    else
        return proNone;
}


unsigned int GaMessage::getAddress() const
{
    return address;
}

unsigned int GaMessage::getPort() const
{
    return port;
}

unsigned int GaMessage::getValue() const
{
    return value;
}

/*
 * class for GL messages
 * Type may be: mtGlInit, mtGlSet, mtGlGet, mtGlTerm, mtGlInfo
 */
/* mtGlInit:
 *   INIT <bus> GL <addr> <prot> <protv> <maxsteps> <fcount>
 *    0     1   2    3      4       5         6         7
 */
GlMessage::GlMessage(Type type, unsigned int abus,
        unsigned int addr, Protocol prot, unsigned int pv,
        unsigned int maxvs, unsigned int fc):
    SrcpMessage(type, abus), address(addr), protocol(prot),
    pversion(pv), maxvsteps(maxvs), fcount(fc), fmap(0)
{
}


/* mtGlSet (all functions):
 *   SET <bus> GL <addr> <drivemode> <currentv> <maxv> <f1> . . <fn>
 *    0    1   2    3         4          5        6      7
 */
GlMessage::GlMessage(Type type, unsigned int abus,  
        unsigned int addr, unsigned int fc, unsigned int dm,
        unsigned int cv, unsigned int mv,
        unsigned int map):
    SrcpMessage(type, abus), address(addr), fcount(fc), drivemode(dm),
    currentv(cv), maxv(mv), fmap(map)
{
}

/* mtGlSet:
 *   SET <bus> GL <addr> <drivemode> <currentv> <maxv> <f1> . . <fn>
 *    0    1   2    3         4          5        6      7
 */
GlMessage::GlMessage(Type type, unsigned int abus, 
        unsigned int addr, unsigned int dm,
        unsigned int cv, unsigned int mv, bool f0):
    SrcpMessage(type, abus), address(addr), drivemode(dm),
    currentv(cv), maxv(mv), fmap(0)
{
    if (f0)
        fmap |= glF0;
    else
        fmap &= ~glF0;
}

void GlMessage::setFunctionMap(unsigned int map = 0)
{
    fmap = map;
}

/* mtGlGet:
 *   GET <bus> GL <addr>
 *    0    1   2    3
 */
/* mtGlTerm:
 *   TERM <bus> GL <addr>
 *    0     1   2    3
 */
GlMessage::GlMessage(Type type, unsigned int abus,     
        unsigned int addr):
    SrcpMessage(type, abus), address(addr)
{
}


/* mtGlInfo 100: (SET response) + fcount
 *    100 INFO <bus> GL <addr> <drivemode> <cv> <maxv> <f0> ...
 *     0   1     2   3    4         5       6     7      8
 */
GlMessage::GlMessage(Type type, Code acode, unsigned int abus,
        unsigned int addr, unsigned int fc, unsigned int dm,
        unsigned int cv, unsigned int mv, unsigned int map):
    SrcpMessage(type, acode, abus), address(addr), fcount(fc),
    drivemode(dm), currentv(cv), maxv(mv), fmap(map)
{
}


/* mtGlInfo 101:  (INIT response)
 *    101 INFO <bus> GL <addr> <prot> <protv> <maxsteps> <fcount>
 *     0    1     2   3    4     5       6        7         8
 */
GlMessage::GlMessage(Type type, Code acode, unsigned int abus,
        unsigned int addr, Protocol prot, unsigned int pv,
        unsigned int maxvs, unsigned int fc):
    SrcpMessage(type, acode, abus), address(addr), protocol(prot),
    pversion(pv), maxvsteps(maxvs), fcount(fc)
{
}


/* mtGlInfo 102: (TERM response)
 *   102 INFO <bus> GL <addr>
 *    0   1     2   3    4
 */
GlMessage::GlMessage(Type type, Code acode, unsigned int abus,
        unsigned int addr):
    SrcpMessage(type, acode, abus), address(addr)
{
}


QString GlMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtGlSet:
            cmdStr = QString("SET %1 GL %2 %3 %4 %5 %6")
                .arg(bus).arg(address).arg(drivemode)
                .arg(currentv).arg(maxv)
                .arg((fmap & glF0) == glF0);

            if (fcount > 1) {
                cmdStr = cmdStr + getF1toF4Appendix();
                if (fcount > 5) {
                    cmdStr = cmdStr + getF5toF8Appendix();
                    if (fcount > 9) {
                        cmdStr = cmdStr + getF9toF12Appendix();
                        if (fcount > 13) {
                            cmdStr = cmdStr + getF13toF16Appendix();
                            if (fcount > 17) {
                                cmdStr = cmdStr + getF17toF20Appendix();
                                if (fcount > 21) {
                                    cmdStr = cmdStr + getF21toF24Appendix();
                                    if (fcount > 25) {
                                        cmdStr = cmdStr + getF25toF28Appendix();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        case mtGlGet:
            cmdStr = QString("GET %1 GL %2")
                .arg(bus).arg(address);
            break;
        case mtGlInit:
            switch (protocol) {
                case proMM:
                    cmdStr = QString("INIT %1 GL %2 %3 %4 %5 %6")
                        .arg(bus).arg(address)
                        .arg(getProtocolStr(protocol))
                        .arg(pversion).arg(maxvsteps)
                        .arg(fcount);
                    break;
                case proDCC:
                    cmdStr = QString("INIT %1 GL %2 %3 %4 %5 %6")
                        .arg(bus).arg(address)
                        .arg(getProtocolStr(protocol))
                        .arg(pversion).arg(maxvsteps)
                        .arg(fcount);
                    break;
                case proMfx:
                    cmdStr = QString("INIT %1 GL %2 %3 %4 %5 %6")	// 123456 \"test\" 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0")
                        .arg(bus).arg(address)
                        .arg(getProtocolStr(protocol))
                        .arg(pversion).arg(maxvsteps)
                        .arg(fcount);
                    break;
                default:
                    cmdStr = QString("INIT %1 GL %2 %3")
                        .arg(bus).arg(address)
                        .arg(getProtocolStr(protocol));
                    break;
            }
            break;
        case mtGlInfo: //FIXME: add
        case mtGlTerm:
            cmdStr = QString("TERM %1 GL %2")
                .arg(bus).arg(address);
            break;
        default:
            break;
    }

    return cmdStr;
}

/*return GL protocol string values*/
QString GlMessage::getProtocolStr(Protocol pro) const
{
    QString proStr = "P";

    switch (pro) {
        case proAnalog:
            proStr = "A";
            break;
        case proFm:
            proStr = "F";
            break;
        case proLn:
            proStr = "L";
            break;
        case proMM:
            proStr = "M";
            break;
        case proDCC:
            proStr = "N";
            break;
        case proServer:
    //        proStr = "P";
            break;
        case proSlx:
            proStr = "S";
            break;
        case proZimo:
            proStr = "Z";
            break;
        case proMfx:
            proStr = "X";
            break;
        default:
            break;
    }
    return proStr;
}

GlMessage::Protocol GlMessage::getProtocolId(const QString& pro)
{
    if (pro == "M")
        return proMM;
    else if (pro == "N")
        return proDCC;
    else if (pro == "P")
        return proServer;
    else if (pro == "L")
        return proLn;
    else if (pro == "S")
        return proSlx;
    else if (pro == "Z")
        return proZimo;
    else if (pro == "A")
        return proAnalog;
    else if (pro == "F")
        return proFm;
    else if (pro == "X")
        return proMfx;
    else
        return proNone;
}


QString GlMessage::getF1toF4Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF1) == glF1)
        .arg((fmap & glF2) == glF2)
        .arg((fmap & glF3) == glF3)
        .arg((fmap & glF4) == glF4);

    return cmdStr;
}


QString GlMessage::getF5toF8Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF5) == glF5)
        .arg((fmap & glF6) == glF6)
        .arg((fmap & glF7) == glF7)
        .arg((fmap & glF8) == glF8);

    return cmdStr;
}


QString GlMessage::getF9toF12Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF9) == glF9)
        .arg((fmap & glF10) == glF10)
        .arg((fmap & glF11) == glF11)
        .arg((fmap & glF12) == glF12);

    return cmdStr;
}


QString GlMessage::getF13toF16Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF13) == glF13)
        .arg((fmap & glF14) == glF14)
        .arg((fmap & glF15) == glF15)
        .arg((fmap & glF16) == glF16);

    return cmdStr;
}


QString GlMessage::getF17toF20Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF17) == glF17)
        .arg((fmap & glF18) == glF18)
        .arg((fmap & glF19) == glF19)
        .arg((fmap & glF20) == glF20);

    return cmdStr;
}


QString GlMessage::getF21toF24Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF21) == glF21)
        .arg((fmap & glF22) == glF22)
        .arg((fmap & glF23) == glF23)
        .arg((fmap & glF24) == glF24);

    return cmdStr;
}


QString GlMessage::getF25toF28Appendix() const
{
    QString cmdStr = QString(" %1 %2 %3 %4")
        .arg((fmap & glF25) == glF25)
        .arg((fmap & glF26) == glF26)
        .arg((fmap & glF27) == glF27)
        .arg((fmap & glF28) == glF28);

    return cmdStr;
}


bool GlMessage::hasAddress(unsigned int addr) const
{
    return (addr == address);
}

//FIXME: isEqual(unsigned int map)
bool GlMessage::matchFunctionMap(unsigned int map) const
{
    return (map == fmap);
}


unsigned int GlMessage::getFunctionMap() const
{
    return fmap;
}


void GlMessage::getDriveSet(unsigned int& dm, unsigned int& cv,
        unsigned int& mv) const
{
    dm = drivemode;
    cv = currentv;
    mv = maxv;
}


/*
 * class for Gm messages
 * type may be: mtGmSet, mtGmInfo
 */
/*
 * mtGmSet:
 *   SET <bus> GM <sendto> <replyto> <gmtype> <content>
 *    0    1   2     3         4         5        6
 */
GmMessage::GmMessage(Type type, unsigned int sto, unsigned int rto,
        const QString& gmt, const QString& cont):
    SrcpMessage(type, 0), sendto(sto), replyto(rto), gmtype(gmt),
    content(cont)
{
}


/*
 * mtGmInfo:
 *   <time> 100 INFO <bus=0> GM <sendto> <replyto> <gmtype> <content>
 *     0     1   2     3      4    5         6        7         8
 */
GmMessage::GmMessage(Type type, Code acode, unsigned int sto, unsigned int rto,
        const QString& gmt, const QString& cont):
    SrcpMessage(type, acode, 0), sendto(sto), replyto(rto), gmtype(gmt),
    content(cont)
{
}


unsigned int GmMessage::getSendId() const
{
    return sendto;
}


unsigned int GmMessage::getReplyId() const
{
    return replyto;
}
QString GmMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtGmSet:
            cmdStr = QString("SET 0 GM %1 %2 %3 %4")
                .arg(sendto).arg(replyto).arg(gmtype).arg(content);
            break;
        case mtGmInfo:
            cmdStr = QString("%1 INFO 0 GM %2 %3 %4 %5")
                .arg(code).arg(sendto).arg(replyto).arg(gmtype).arg(content);
            break;
        default:
            break;
    }

    return cmdStr;
}

/*parse content string and return CRCF message pointer*/
CrcfMessage* GmMessage::createCrcfMessage() const
{
    return CrcfMessage::parse(content);
}


/*
 * class for Lock messages
 * type may be: mtLockSet, mtLockGet, mtLockTerm, mtLockInfo
 */
/*
 * mtLockGet:
 *   GET <bus> LOCK <devicegroup> <address>
 *    0    1     2        3           4
 */
LockMessage::LockMessage(Type type, unsigned int abus, Device adevice,
        unsigned int anaddress):
    SrcpMessage(type, abus), device(adevice), address(anaddress)
{
}

/*
 * mtLockSet:
 *   GET <bus> LOCK <devicegroup> <address> <duration>
 *    0    1     2        3           4         5
 */
LockMessage::LockMessage(Type type, unsigned int abus, Device adevice,
        unsigned int anaddress, unsigned int aduration):
    SrcpMessage(type, abus), device(adevice), address(anaddress),
    duration(aduration)
{
}


/*
 * mtLockInfo: 101/102
 *   <code> INFO <bus> LOCK <devicegroup> <address>
 *     0     1     2    3         4           5
 */
LockMessage::LockMessage(Type type, Code acode, unsigned int abus,
        Device adevice, unsigned int anaddress):
    SrcpMessage(type, acode, abus), device(adevice), address(anaddress)
{
}


/*
 * mtLockInfo: 100
 *   <code> INFO <bus> LOCK <devicegroup> <address> <duration> <sessionid>
 *     0     1     2    3         4           5         6           7
 */
LockMessage::LockMessage(Type type, Code acode, unsigned int abus,
        Device adevice, unsigned int anaddress, unsigned int aduration,
        unsigned int asessionid):
    SrcpMessage(type, acode, abus), device(adevice), address(anaddress),
    duration(aduration), session(asessionid)
{
}


QString LockMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtLockSet:
            cmdStr = QString("SET %1 LOCK %2 %3 %4")
                .arg(bus).arg(getDeviceStr(device)).arg(address).arg(duration);
            break;
        case mtLockGet:
            cmdStr = QString("GET %1 LOCK %2 %3")
                .arg(bus).arg(getDeviceStr(device)).arg(address);
            break;
        case mtLockTerm:
            cmdStr = QString("TERM %1 LOCK %2 %3")
                .arg(bus).arg(getDeviceStr(device)).arg(address);
            break;
        case mtLockInfo:
            switch (code) {
                case code100:
                    cmdStr = QString("%1 INFO %2 LOCK %3 %4 %5 %6")
                        .arg(code).arg(bus).arg(getDeviceStr(device))
                        .arg(address).arg(duration).arg(session);
                        break;
                case code101:
                case code102:
                    cmdStr = QString("%1 INFO %2 LOCK %3 %4").arg(code)
                        .arg(bus).arg(getDeviceStr(device)).arg(address);
                        break;
                default:
                        break;
            }
            break;
        default:
            break;
    }

    return cmdStr;
}


QString SrcpMessage::getDeviceStr(Device device) const
{
    QString dev = "";

    switch(device) {
        case SrcpMessage::devFb:
            dev = QString("FB");
            break;
        case SrcpMessage::devGa:
            dev = QString("GA");
            break;
        case SrcpMessage::devGl:
            dev = QString("GL");
            break;
        case SrcpMessage::devGm:
            dev = QString("GM");
            break;
        case SrcpMessage::devLock:
            dev = QString("LOCK");
            break;
        case SrcpMessage::devPower:
            dev = QString("POWER");
            break;
        case SrcpMessage::devServer:
            dev = QString("SERVER");
            break;
        case SrcpMessage::devSession:
            dev = QString("SESSION");
            break;
        case SrcpMessage::devSm:
            dev = QString("SM");
            break;
        case SrcpMessage::devTime:
            dev = QString("TIME");
            break;
        default:
            break;
    }

    return dev;
}

SrcpMessage::Device SrcpMessage::getDeviceId(const QString& dev)
{
    if (dev == "FB")
        return SrcpMessage::devFb;
    else if (dev == "GA")
        return SrcpMessage::devGa;
    else if (dev == "GL")
        return SrcpMessage::devGl;
    else if (dev == "GM")
        return SrcpMessage::devGm;
    else if (dev == "LOCK")
        return SrcpMessage::devLock;
    else if (dev == "POWER")
        return SrcpMessage::devPower;
    else if (dev == "SERVER")
        return SrcpMessage::devServer;
    else if (dev == "SESSION")
        return SrcpMessage::devSession;
    else if (dev == "SM")
        return SrcpMessage::devSm;
    else if (dev == "TIME")
        return SrcpMessage::devTime;
    else
        return SrcpMessage::devNone;
}

/*
 * class for Power messages
 * type may be: mtPowerInit, mtPowerSet, mtPowerGet, mtPowerTerm,
 *              mtPowerInfo
 */

/*
 * mtPowerInit:
 *   INIT <bus> POWER
 *    0    1     2
 * mtPowerTerm:
 *   TERM <bus> POWER
 *    0    1     2
 * mtPowerGet:
 *   GET <bus> POWER
 *    0    1     2
 */
PowerMessage::PowerMessage(Type type, unsigned int abus):
    SrcpMessage(type, abus), poweron(false)
{
}

/* mtPowerSet:
 *   GET <bus> POWER <on/off>
 *    0    1     2    3
 */

PowerMessage::PowerMessage(Type type, unsigned int abus, bool apoweron):
    SrcpMessage(type, abus), poweron(apoweron)
{
}

/* mtPowerInfo:
 * <time> <100> INFO <bus> POWER <on/off>
 *   0      1    2     3    4       5
 */
PowerMessage::PowerMessage(Type type, Code acode, unsigned int abus,
        bool apower):
    SrcpMessage(type, acode, abus), poweron(apower)
{
}


QString PowerMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtPowerSet: //FIXME: text is missing
            cmdStr = QString("SET %1 POWER %2")
                .arg(bus).arg((poweron) ? "ON" : "OFF");
            break;
        case mtPowerGet:
            cmdStr = QString("GET %1 POWER").arg(bus);
            break;
        case mtPowerInfo: //FIXME: text is missing+ TERM
            if (code == code100)
                cmdStr = QString("100 INFO %1 POWER %2")
                    .arg(bus).arg((poweron) ? "ON" : "OFF");
            break;
        case mtPowerInit:
            cmdStr = QString("INIT %1 POWER").arg(bus);
            break;
        case mtPowerTerm:
            cmdStr = QString("TERM %1 POWER").arg(bus);
            break;
        default:
            break;
    }

    return cmdStr;
}


bool PowerMessage::ison() const
{
    return poweron;
}

int PowerMessage::busison(unsigned int abus)
{
	if (bus == abus) return poweron;
	else return -1;
}

/*
 * class for Server messages
 * type may be: mtServerReset, mtServerTerm, mtServerInfo
 */
ServerMessage::ServerMessage(Type type):
    SrcpMessage(type, 0)
{
}


/* <time> 101/2 INFO 0 SERVER*/
ServerMessage::ServerMessage(Type type, Code acode):
    SrcpMessage(type, acode, 0)
{
}


/* <time> 100 INFO 0 SERVER <state>*/
ServerMessage::ServerMessage(Type type, Code acode,
        ServerMessage::ServerState astate):
    SrcpMessage(type, acode, 0), state(astate)
{
}

ServerMessage::ServerState ServerMessage::getState() const
{
    return state;
}


QString ServerMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtServerGet:
            cmdStr = QString("GET 0 SERVER");
            break;
        case mtServerReset:
            cmdStr = QString("RESET 0 SERVER");
            break;
        case mtServerInfo:
            switch (code) {
                case SrcpMessage::code100:
                    cmdStr = QString("%1 INFO 0 SERVER %2")
                        .arg(code).arg(getServerStateStr(state));
                    break;
                case SrcpMessage::code101:
                    cmdStr = QString("%1 INFO 0 SERVER").arg(code);
                    break;
                case SrcpMessage::code102:
                    cmdStr = QString("%1 INFO 0 SERVER") .arg(code);
                    break;
                default:
                    break;
            }
            break;
        case mtServerTerm:
            cmdStr = QString("TERM 0 SERVER");
            break;
        default:
            break;
    }

    return cmdStr;
}

/*return Server state string values*/
QString ServerMessage::getServerStateStr(ServerMessage::ServerState state) const
{
    QString stateStr = "";

    switch (state) {
        case ssRunning:
            stateStr = "RUNNING";
            break;
        case ssResetting:
            stateStr = "RESETTING";
            break;
        case ssTerminating:
            stateStr = "TERMINATING";
            break;
        default:
            break;
    }
    return stateStr;
}

/*
 * class for Session messages
 * type may be: mtSessionGet, mtSessionTerm, mtSessionInfo
 * TERM 0 SESSION
 */
SessionMessage::SessionMessage(Type type):
    SrcpMessage(type, code0, 0), id(0)
{
}


/*
 * TERM 0 SESSION <id>
 * GET 0 SESSION <id>
 */
SessionMessage::SessionMessage(Type type, unsigned int anid):
    SrcpMessage(type, code0, 0), id(anid)
{
}


/*
 * <code> INFO 0 SESSION <id>
 */
SessionMessage::SessionMessage(Type type, Code acode, unsigned int anid):
    SrcpMessage(type, acode, 0), id(anid)
{
}


QString SessionMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtSessionGet:
            cmdStr = QString("GET 0 SESSION %1").arg(id);
            break;
        case mtSessionInfo:
            switch(code) {
                case code100:
                    cmdStr = QString("%1 INFO 0 SESSION %2").arg(code)
                        .arg(id);
                    break;
                case code101:
                    cmdStr = QString("%1 INFO 0 SESSION %2").arg(code)
                        .arg(id);
                    break;
                case code102:
                    cmdStr = QString("%1 INFO 0 SESSION %2").arg(code)
                        .arg(id);
                    break;
                default:
                    break;
            }
            break;
        case mtSessionTerm:
            if (id == 0)
                cmdStr = QString("TERM 0 SESSION");
            else
                cmdStr = QString("TERM 0 SESSION %1").arg(id);
            break;
        default:
            break;
    }

    return cmdStr;
}


/*
 * class for Sm messages
 * type may be: mtSmGet, mtSmSet, mtSmInit, mtSmTerm, mtSmVerify, mtSmInfo
 * TERM <bus> SM
 */
SmMessage::SmMessage(Type atype, unsigned int abus):
    SrcpMessage(atype, abus), smprotocol(smpNmra)
{
}


/*
 * INIT <bus> SM <protocol>
 */
SmMessage::SmMessage(Type atype, unsigned int abus, int smp):
    SrcpMessage(atype, abus), smprotocol(smp)
{
}

/*
 * GET <bus> SM <decaddress> <smtype> <smaddress> 
 */
SmMessage::SmMessage(Type atype, unsigned int abus,
        unsigned int adecaddress, SmType asmtype, unsigned int anaddress):
    SrcpMessage(atype, abus), smprotocol(smpNmra), decaddress(adecaddress),
    smtype(asmtype), address(anaddress)
{
}

/*
 * SET/VERIFY <bus> SM <decaddress> <smtype> <smaddress> <smvalue>
 */
SmMessage::SmMessage(Type atype, unsigned int abus,
        unsigned int adecaddress, SmType asmtype, unsigned int anaddress,
        unsigned int avalue):
    SrcpMessage(atype, abus), smprotocol(smpNmra), decaddress(adecaddress),
    smtype(asmtype), address(anaddress), value(avalue)
{
}

/*
 * SET/GET/VERiFY <bus> SM <decaddress> CVBIT <smaddress> <bit> <smvalue>
 * for GET avalue should be 0
 */
SmMessage::SmMessage(Type atype, unsigned int abus,
        unsigned int adecaddress, SmType asmtype, unsigned int anaddress,
        unsigned int abit, unsigned int avalue):
    SrcpMessage(atype, abus), smprotocol(smpNmra), decaddress(adecaddress),
    smtype(asmtype), address(anaddress), bit(abit), value(avalue)
{
}

/* mtSmInfo
   <time> 102 INFO <bus> SM
 */
SmMessage::SmMessage(Type atype, Code acode, unsigned int abus):
    SrcpMessage(atype, acode, abus)
{
}


/* mtSmInfo
   <time> 101 INFO <bus> SM <prot>
 */
SmMessage::SmMessage(Type atype, Code acode, unsigned int abus,
        int smp):
    SrcpMessage(atype, acode, abus), smprotocol(smp)
{
}

/* mtSmInfo
   <time> 100 INFO <bus> SM <addr> CV <cv_addr> <cv_value>
   <time> 100 INFO <bus> SM <addr> REG <reg_addr> <reg_value>
   <time> 100 INFO <bus> SM <addr> PAGE <page_addr> <page_value>
 */
SmMessage::SmMessage(Type atype, Code acode, unsigned int abus,
        unsigned int addr, SmType asmtype, unsigned int sma,
        unsigned int avalue):
    SrcpMessage(atype, acode, abus), decaddress(addr),
    smtype(asmtype), address(sma), value(avalue)
{
}


/* mtSmInfo
   <time> 100 INFO <bus> SM <addr> CVBIT <cvb_addr> <cvb_bit> <cvb_value>
 */
SmMessage::SmMessage(Type atype, Code acode, unsigned int abus,
        unsigned int addr, SmType asmtype, unsigned int sma,
        unsigned int abit, unsigned int avalue):
    SrcpMessage(atype, acode, abus), decaddress(addr),
    smtype(asmtype), address(sma), bit(abit), value(avalue)
{
}


int SmMessage::getSmProtocolId(const QString& prot)
{
    if (prot == "NMRA")
        return smpNmra;
    else
        return smpNone;
}


SmMessage::SmType SmMessage::getSmTypeId(const QString& smt)
{
    if (smt == "CV")
        return smtCv;
    else if (smt == "CVBIT")
        return smtCvbit;
    else if (smt == "REG")
        return smtReg;
    else if (smt == "PAGE")
        return smtPage;
    else if (smt == "CVMFX")
        return smtCvmfx;
    else if (smt == "BIND")
        return smtBind;
    else
        return smtNone;
}


QString SmMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtSmGet:
            switch(smtype) {
                case smtCv:
                    cmdStr = QString("GET %1 SM %2 CV %3")
                        .arg(bus).arg(decaddress).arg(address);
                    break;
                case smtCvbit:
                    cmdStr = QString("GET %1 SM %2 CVBIT %3 %4")
                        .arg(bus).arg(decaddress).arg(address)
                        .arg(bit);
                    break;
                case smtReg:
                    cmdStr = QString("GET %1 SM %2 REG %3")
                        .arg(bus).arg(decaddress).arg(address);
                    break;
                case smtPage:
                    cmdStr = QString("GET %1 SM %2 PAGE %3")
                        .arg(bus).arg(decaddress).arg(address);
                    break;
                default:
                    break;
            }
            break;

        case mtSmSet:
            switch(smtype) {
                case smtCv:
                    cmdStr = QString("SET %1 SM %2 CV %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                case smtCvbit:
                    cmdStr = QString("SET %1 SM %2 CVBIT %3 %4 %5")
                        .arg(bus).arg(decaddress).arg(address)
                        .arg(bit).arg(value);
                    break;
                case smtReg:
                    cmdStr = QString("SET %1 SM %2 REG %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                case smtPage:
                    cmdStr = QString("SET %1 SM %2 PAGE %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                case smtCvmfx:		// FIXME: CVMFX <cvline> <cvindex> <value>
                    cmdStr = QString("SET %1 SM %2 CVMFX %3 %4 %5")
                        .arg(bus).arg(decaddress).arg(address)
                        .arg(bit).arg(value);
                    break;
				case smtBind:		
                    cmdStr = QString("SET %1 SM %2 BIND %3")
                        .arg(bus).arg(decaddress).arg(address);
                    break;
                default:
                    break;
            }
            break;

        case mtSmVerify:
            switch(smtype) {
                case smtCv:
                    cmdStr = QString("VERIFY %1 SM %2 CV %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                case smtCvbit:
                    cmdStr = QString("VERIFY %1 SM %2 CVBIT %3 %4 %5")
                        .arg(bus).arg(decaddress).arg(address)
                        .arg(bit).arg(value);
                    break;
                case smtReg:
                    cmdStr = QString("VERIFY %1 SM %2 REG %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                case smtPage:
                    cmdStr = QString("VERIFY %1 SM %2 PAGE %3 %4")
                        .arg(bus).arg(decaddress).arg(address).arg(value);
                    break;
                default:
                    break;
            }
            break;

        case mtSmInit:
            cmdStr = QString("INIT %1 SM %2").arg(bus)
                .arg(getSmProtocolStr());
            break;

        case mtSmTerm:
            cmdStr = QString("TERM %1 SM").arg(bus);
            break;

        case mtSmInfo:
            switch(code) {
                case code100:
                    switch(smtype) {
                        case smtCv:
                            cmdStr = QString("%1 INFO %2 SM %3 CV %4 %5")
                                .arg(code).arg(bus)
                                .arg(decaddress).arg(address).arg(value);
                            break;
                        case smtCvbit:
                            cmdStr = QString("%1 INFO %2 SM %3 CVBIT %4 %5 %6")
                                .arg(code).arg(bus).arg(decaddress)
                                .arg(address).arg(bit).arg(value);
                            break;
                        case smtReg:
                            cmdStr = QString("%1 INFO %2 SM %3 REG %4 %5")
                                .arg(code).arg(bus)
                                .arg(decaddress).arg(address).arg(value);
                            break;
                        case smtPage:
                            cmdStr = QString("%1 INFO %2 SM %3 PAGE %4 %5")
                                .arg(code).arg(bus)
                                .arg(decaddress).arg(address).arg(value);
                            break;
                        default:
                            break;
                    }
                    break;
                case code101:
                    cmdStr = QString("%1 INFO %2 SM %3").arg(code)
                            .arg(bus).arg(getSmProtocolStr());
                    break;
                case code102:
                    cmdStr = QString("%1 INFO %2 SM").arg(code).arg(bus);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return cmdStr;
}


QString SmMessage::getSmProtocolStr() const
{
    QString protocolStr = "";

    switch (smprotocol) {
        case smpNmra:
            protocolStr = "NMRA";
            break;
        default:
            break;
    }
    return protocolStr;
}

/*
 * class for Time messages
 * type may be: mtTimeGet, mtTimeSet, mtTimeInit, mtTimeTerm, mtTimeInfo
 * GET 0 TIME
 */
TimeMessage::TimeMessage(Type atype):
    SrcpMessage(atype, 0)
{
}


/*
 * 102 INFO 0 TIME
 */
TimeMessage::TimeMessage(Type atype, Code acode):
    SrcpMessage(atype, acode, 0)
{
}


/*
 * INIT 0 TIME <fx> <fy>
 */
TimeMessage::TimeMessage(Type atype, unsigned int afx, unsigned int afy):
    SrcpMessage(atype, 0), fx(afx), fy(afy)
{
}


/*
 * 101 INFO 0 TIME <fx> <fy>
 */
TimeMessage::TimeMessage(Type atype, Code acode, unsigned int afx,
        unsigned int afy):
    SrcpMessage(atype, acode, 0), fx(afx), fy(afy)
{
}


/*
 * SET/WAIT 0 TIME <d> <h> <m> <s>
 */
TimeMessage::TimeMessage(Type atype, unsigned int ad, unsigned int ah,
        unsigned int am, unsigned int as):
    SrcpMessage(atype, 0), d(ad), h(ah), m(am), s(as)
{
}


/*
 * 100 INFO 0 TIME <d> <h> <m> <s>
 */
TimeMessage::TimeMessage(Type atype, Code acode, unsigned int ad,
        unsigned int ah, unsigned int am, unsigned int as):
    SrcpMessage(atype, acode, 0), d(ad), h(ah), m(am), s(as)
{
}


QString TimeMessage::getSrcpMessageStr() const
{
    QString cmdStr = "";

    switch (type) {
        case mtTimeGet:
            cmdStr = QString("GET 0 TIME");
            break;
        case mtTimeSet:
            cmdStr = QString("SET 0 TIME %1 %2 %3 %4")
                .arg(d).arg(h).arg(m).arg(s);
            break;
        case mtTimeWait:
            cmdStr = QString("WAIT 0 TIME %1 %2 %3 %4")
                .arg(d).arg(h).arg(m).arg(s);
            break;
        case mtTimeInit:
            cmdStr = QString("INIT 0 TIME %1 %2").arg(fx).arg(fy);
            break;
        case mtTimeTerm:
            cmdStr = QString("TERM 0 TIME");
            break;
        case mtTimeInfo:
            switch(code) {
                case code100:
                    cmdStr = QString("%1 INFO 0 TIME %2 %4 %5 %6")
                        .arg(code).arg(d).arg(h).arg(m).arg(s);
                    break;
                case code101:
                    cmdStr = QString("%1 INFO 0 TIME %2 %3").arg(code)
                        .arg(fx).arg(fy);
                    break;
                case code102:
                    cmdStr = QString("%1 INFO 0 TIME").arg(code);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return cmdStr;
}
