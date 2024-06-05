// srcpmessage.h - adapted for raidtcl project 2018 by Rainer Müller 

/***************************************************************************
 srcpmessage.h
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
    This is the headerfile for srcpmessage.cpp
 ***************************************************************************/

#ifndef SRCPMESSAGE_H
#define SRCPMESSAGE_H

#include <QString>

#include "crcfmessage.h"


class SrcpMessage
{
    public:
        enum Device {
            devNone = 0,
            devFb = 1,
            devGa = 2,
            devGl = 4,
            devGm = 8,
            devLock = 16,
            devPower = 32,
            devServer = 64,
            devSession = 128,
            devSm = 256,
            devTime = 512
        };

        enum Type {mtNone = 0,
            mtAckHandShake, mtAckCommand,
            mtDescriptionGet, mtDescriptionInfo,
            mtFbInit, mtFbSet, mtFbGet, mtFbTerm, mtFbInfo,
            mtGaInit, mtGaSet, mtGaGet, mtGaTerm, mtGaInfo,
            mtGlInit, mtGlSet, mtGlGet, mtGlTerm, mtGlInfo,
            mtGmSet, mtGmInfo,
            mtLockSet, mtLockGet, mtLockTerm, mtLockInfo,
            mtPowerInfo, mtPowerInit, mtPowerSet, mtPowerGet, mtPowerTerm,
            mtResponse,
            mtServerGet, mtServerReset, mtServerTerm, mtServerInfo,
            mtSessionGet, mtSessionTerm, mtSessionInfo,
            mtSmSet, mtSmGet, mtSmInit, mtSmTerm, mtSmVerify, mtSmInfo,
            mtTimeGet, mtTimeSet, mtTimeInit, mtTimeTerm, mtTimeWait,
            mtTimeInfo
        };

        enum Code {code0 = 0,
            code100 = 100, code101, code102, code103,
            code200 = 200, code201, code202,
            code410 = 410, code411, code412,
            code414 = 414, code415, code416, code417, code418, code419,
            code420, code421, code422, code423, code424, code425,
            code499 = 499
        };

        SrcpMessage(Type, unsigned int abus);
        SrcpMessage(Type, Code acode, unsigned int abus);
        virtual ~SrcpMessage();
        virtual QString getSrcpMessageStr() const = 0;
        //virtual SrcpMessage* parse(QString&) = 0;
        void setType(SrcpMessage::Type); //only valid types are allowed
        Type getType() const;
        void setCode(SrcpMessage::Code);
        SrcpMessage::Code getCode() const;
        void setBus(unsigned int);
        unsigned int getBus() const;
        static SrcpMessage::Device getDeviceId(const QString&);

    protected:
        Type type;
        Code code;
        unsigned int bus;
        QString getDeviceStr(const Device dev) const;
};


class AckMessage: public SrcpMessage
{
    public:
        /* mtAckOk */
        AckMessage(Type type, Code acode);
        /* mtAckOk GO <sid>*/
        AckMessage(Type type, Code acode, unsigned int sid);
        /* mtAckOk GO <protocol>*/
        /* mtAckError <text>*/
        AckMessage(Type type, Code acode, const QString& txt);
        unsigned int getSessionId();
        QString getText() const;

    protected:
        unsigned int sessionid;
        QString text;
        QString getSrcpMessageStr() const;
};


class DescriptionMessage: public SrcpMessage
{
        QString getDevicelistStr() const;

    public:
        /* mtDescriptionGet */
        DescriptionMessage(Type type, unsigned int abus);
        /* mtDescriptionInfo 100 */
        DescriptionMessage(Type type, Code acode, unsigned int abus, int
                adevicemap);
        // missing: device description
        bool hasGa() const;
        bool hasGl() const;
        bool hasGm() const;
        bool hasPower() const;

    protected:
        unsigned int devicemap;
        unsigned int address;
        QString getSrcpMessageStr() const;
};


class FbMessage: public SrcpMessage
{
    public:
        /* mtFbGet */
        FbMessage(Type type, unsigned int abus, unsigned int anaddress);
        /* mtFbSet */
        FbMessage(Type type, unsigned int abus, unsigned int anaddress,
                unsigned int avalue);
        /* mtFbInfo 100 */
        FbMessage(Type type, Code acode, unsigned int abus,
                unsigned int anaddress, unsigned int avalue);
        /* mtFbInfo 101/102 */
        FbMessage(Type type, Code acode, unsigned int abus);
        /* mtFbInfo 103 (BiDi)*/
        FbMessage(Type type, Code acode, unsigned int abus,
                unsigned int addr, unsigned int avalue, Device abididev,
                unsigned int gladdr, unsigned int gldm, unsigned int glv);
        unsigned int getAddress() const;
        unsigned int getGlAddress() const;
        unsigned int getValue() const;

    protected:
        unsigned int address;
        unsigned int value;
        Device bididev;
        unsigned int gladdress;
        unsigned int gldrivemode;
        unsigned int glvelocity;
        QString getSrcpMessageStr() const;
};


class GaMessage: public SrcpMessage
{
    public:
        enum Protocol {proNone = 0, proMM, proDCC, proServer, proSelectrix};

        /* mtGaInit */
        GaMessage(Type type, unsigned int abus,
                unsigned int anaddress, Protocol pro);
        /* mtGaTerm */
        GaMessage(Type type, unsigned int abus,
                unsigned int anaddress);
        /* mtGaGet */
        GaMessage(Type type, unsigned int abus, unsigned int anaddress,
                Protocol aprot, unsigned int aport);
        /* mtGaSet */
        GaMessage(Type type, unsigned int abus, unsigned int anaddress,
                Protocol aprot, unsigned int aport, unsigned int avalue,
                unsigned int adelay);
        /* mtGaInfo 100 */
        GaMessage(Type type, Code acode, unsigned int abus,
                unsigned int anaddress, Protocol aprot, unsigned int aport,
                unsigned int avalue);
        /* mtGaInfo 101 */
        GaMessage(Type type, Code acode, unsigned int abus,
                unsigned int anaddress, Protocol pro);
        /* mtGaInfo 102 */
        GaMessage(Type type, Code acode, unsigned int abus,
                unsigned int anaddress);
        static GaMessage::Protocol getProtocolId(const QString& pro);
        unsigned int getAddress() const;
        unsigned int getValue() const;
        unsigned int getPort() const;

    protected:
        unsigned int address;
        Protocol protocol;
        unsigned int port;
        unsigned int value;
        unsigned int delay;
        QString getSrcpMessageStr() const;

    private:
        QString getProtocolStr(GaMessage::Protocol) const;
};

/* GL message class for 1 function */
class GlMessage: public SrcpMessage
{
    public:
        enum Protocol {proNone = 0, proAnalog, proFm, proLn, proMM, proDCC,
            proServer, proSlx, proZimo, proMfx};

        enum fbits {glF0 = 1,
            glF1 = 1 << 1,
            glF2 = 1 << 2,
            glF3 = 1 << 3,
            glF4 = 1 << 4,
            glF5 = 1 << 5,
            glF6 = 1 << 6,
            glF7 = 1 << 7,
            glF8 = 1 << 8,
            glF9 = 1 << 9,
            glF10 = 1 << 10,
            glF11 = 1 << 11,
            glF12 = 1 << 12,
            glF13 = 1 << 13,
            glF14 = 1 << 14,
            glF15 = 1 << 15,
            glF16 = 1 << 16,
            glF17 = 1 << 17,
            glF18 = 1 << 18,
            glF19 = 1 << 19,
            glF20 = 1 << 20,
            glF21 = 1 << 21,
            glF22 = 1 << 22,
            glF23 = 1 << 23,
            glF24 = 1 << 24,
            glF25 = 1 << 25,
            glF26 = 1 << 26,
            glF27 = 1 << 27,
            glF28 = 1 << 28
            };

        /* mtGlInit */
        GlMessage(Type type, unsigned int abus, unsigned int addr,
                Protocol prot, unsigned int pv, unsigned int maxvs,
                unsigned int fc);
        /* mtGlSet (all funtions)*/
        GlMessage(Type type, unsigned int abus,
                unsigned int addr, unsigned int fc, unsigned int dm,
                unsigned int cv, unsigned int mv,
                unsigned int map);
        /* mtGlSet (one funtion) FIXME: fcount missing*/
        GlMessage(Type type, unsigned int abus,
                unsigned int addr, unsigned int dm,
                unsigned int cv, unsigned int mv, bool f0);
        /* mtGlSet extensions 
        void setF1toF4(bool, bool, bool, bool);
        void setF5toF8(bool, bool, bool, bool);
        void setF9toF12(bool, bool, bool, bool);
        void setF13toF16(bool, bool, bool, bool);
        void setF17toF20(bool, bool, bool, bool);
        void setF21toF24(bool, bool, bool, bool);
        void setF25toF28(bool, bool, bool, bool); */
        void setFunctionMap(unsigned int map);    
        /* mtGlTerm */
        GlMessage(Type type, unsigned int abus, unsigned int addr);
        /* mtGlInfo 100 */
        GlMessage(Type type, Code acode, unsigned int abus,
                unsigned int addr, unsigned int fc, unsigned int dm,
                unsigned int cv, unsigned int mv, unsigned int map);
        /* mtGlInfo 101 */
        GlMessage(Type type, Code acode, unsigned int abus,
                unsigned int addr, Protocol prot, unsigned int pv,
                unsigned int maxvs, unsigned int fc);
        /* mtGlInfo 102 */
        GlMessage(Type type, Code acode, unsigned int abus,
                unsigned int addr);
        static GlMessage::Protocol getProtocolId(const QString& pro);
        bool hasAddress(unsigned int addr) const;
        bool matchFunctionMap(unsigned int map) const;
        unsigned int getFunctionMap() const;
        void getDriveSet(unsigned int&, unsigned int&, unsigned int&) const;

    protected:
        unsigned int address;
        Protocol protocol;
        unsigned int pversion;
        unsigned int maxvsteps;
        unsigned int fcount;
        unsigned int drivemode;
        unsigned int currentv;
        unsigned int maxv;
        unsigned int fmap;

        QString getSrcpMessageStr() const;
        QString getF1toF4Appendix() const;
        QString getF5toF8Appendix() const;
        QString getF9toF12Appendix() const;
        QString getF13toF16Appendix() const;
        QString getF17toF20Appendix() const;
        QString getF21toF24Appendix() const;
        QString getF25toF28Appendix() const;

    private:
        QString getProtocolStr(Protocol) const;
};


class GmMessage: public SrcpMessage
{
    unsigned int sendto;
    unsigned int replyto;
    QString gmtype;
    QString content;

    protected:
        QString getSrcpMessageStr() const;

    public:
        /* mtGmSet*/
        /*SET <bus> GM <sendto> <replyto> <gmtype> <content>*/
        GmMessage(Type type, unsigned int sto, unsigned int rto,
                const QString& gmt, const QString& cont);
        /* mtGmInfo*/
        GmMessage(Type type, Code acode, unsigned int sto, unsigned int rto,
                const QString& gmt, const QString& cont);

        unsigned int getSendId() const;
        unsigned int getReplyId() const;
        CrcfMessage* createCrcfMessage() const;
};


class LockMessage: public SrcpMessage
{
        Device device;
        unsigned int address;
        unsigned int duration;
        unsigned int session;

    public:
        /* mtLockGet , mtLockTerm*/
        /*GET <bus> LOCK <devicegroup> <addr>*/
        LockMessage(Type type, unsigned int abus, Device adevice,
                unsigned int anaddress);
        /* mtLockSet */
        /*SET <bus> LOCK <device group> <addr> <duration> */
        LockMessage(Type type, unsigned int abus, Device adevice,
                unsigned int anaddress, unsigned int aduration);
        /* mtLockInfo 101/102 */
        /*101 INFO <bus> LOCK <device group> <addr>*/
        LockMessage(Type type, Code acode, unsigned int abus, Device
                adevice, unsigned int anaddress);
        /* mtLockInfo 100 */
        /*100 INFO <bus> LOCK <device group> <addr> <duration> <sessionid>*/
        LockMessage(Type type, Code acode, unsigned int abus, Device
                adevice, unsigned int anaddress, unsigned int aduration,
                unsigned int asessionid);

    protected:
        QString getSrcpMessageStr() const;
};


class PowerMessage: public SrcpMessage
{
    bool poweron;

    public:
        PowerMessage(Type, unsigned int abus);
        PowerMessage(Type, unsigned int abus, bool apower);
        PowerMessage(Type, Code acode, unsigned int abus, bool
                apower);
        bool ison() const;
        int	 busison(unsigned int abus);

    protected:
        QString getSrcpMessageStr() const;
};


class ResponseMessage: public SrcpMessage
{
    unsigned int error;
};


class ServerMessage: public SrcpMessage
{
    public:
        enum ServerState {ssNone = 0, ssRunning, ssResetting, ssTerminating};

        ServerMessage(Type);
        ServerMessage(Type, Code acode);
        ServerMessage(Type, Code acode, ServerMessage::ServerState astate);
        ServerMessage::ServerState getState() const;

    protected:
        QString getSrcpMessageStr() const;

    private:
        ServerMessage::ServerState state;
        QString getServerStateStr(ServerMessage::ServerState) const;

};


class SessionMessage: public SrcpMessage
{
    unsigned int id;

    public:
        SessionMessage(Type);
        SessionMessage(Type, unsigned int anid);
        SessionMessage(Type, Code acode, unsigned int anid);

    protected:
        QString getSrcpMessageStr() const;
};


class SmMessage: public SrcpMessage
{
    public:
        enum SmType {smtNone = 0, smtCv, smtCvbit, smtReg, smtPage, smtCvmfx, smtBind};
        enum SmProtocol {smpNone = 0, smpNmra};

        SmMessage(Type atype, unsigned int abus);
        SmMessage(Type atype, unsigned int abus, int smp);
        SmMessage(Type atype, unsigned int abus, unsigned int adecaddress,
                SmType asmtype, unsigned int anaddress);
        SmMessage(Type atype, unsigned int abus, unsigned int adecaddress,
                SmType asmtype, unsigned int anaddress, unsigned int avalue);
        SmMessage(Type atype, unsigned int abus, unsigned int adecaddress,
                SmType asmtype, unsigned int anaddress, unsigned int abit,
                unsigned int avalue);
        /*mtSmInfo variants*/
        SmMessage(Type atype, Code acode, unsigned int abus);
        SmMessage(Type atype, Code acode, unsigned int abus, int smp);
        SmMessage(Type atype, Code acode, unsigned int abus,
                unsigned int addr, SmType asmtype,
                unsigned int sma, unsigned int avalue);
        SmMessage(Type atype, Code acode, unsigned int abus,
                unsigned int addr, SmType asmtype,
                unsigned int abit, unsigned int sma,
                unsigned int avalue);
        static int getSmProtocolId(const QString&);
        static SmMessage::SmType getSmTypeId(const QString&);

    protected:
        QString getSrcpMessageStr() const;

    private:
        int smprotocol;
        unsigned int decaddress;
        SmType smtype;
        unsigned int address, bit, value;
        QString getSmProtocolStr() const;
};


class TimeMessage: public SrcpMessage
{
    unsigned int fx, fy, d, h, m, s;

    public:
        TimeMessage(Type atype);
        TimeMessage(Type atype, Code acode);
        TimeMessage(Type atype, unsigned int afx, unsigned int afy);
        TimeMessage(Type atype, Code acode, unsigned int afx,
                unsigned int afy);
        TimeMessage(Type atype, unsigned int ax, unsigned int ay,
                unsigned int am, unsigned int as);
        TimeMessage(Type atype, Code acode, unsigned int ax,
                unsigned int ay, unsigned int am, unsigned int as);

    protected:
        QString getSrcpMessageStr() const;
};


#endif

