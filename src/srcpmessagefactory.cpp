// srcpmessagefactory.cpp - adapted for raidtcl project 2019 by Rainer Müller 

/***************************************************************************
 srcpmessagefactory.cpp
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
    This code implements a parser class for SRCP messages which can be
    exchanged between a SRCP-data containing class and the SRCP protocol
    class. It implements a method to translate SRCP-protocol dependend
    command strings to binary data.
 ***************************************************************************/

#include <QStringList>

#include "srcpmessagefactory.h"


SrcpMessageFactory::SrcpMessageFactory(QObject* parent)
        : QObject(parent), error("")
{
}


QString SrcpMessageFactory::getErrorStr() const
{
    return error;
}


SrcpMessage* SrcpMessageFactory::parse(const QString& line)
{
    QStringList tokens = line.split(' ');

    /*
     * OK
     *
     * <time> 200 OK GO             <sessionid>
     * <time> 201 OK PROTOCOL       <protocol>
     * <time> 202 OK CONNECTIONMODE
     *   0     1  2    3                4
     */
    SrcpMessage::Code code = (SrcpMessage::Code) tokens[1].toUInt();
    if (tokens[2] == "OK") {
        // handshake
        if (tokens.size() > 3) {
            if (tokens[3] == "GO") {
                if (tokens.size() > 4) {
                    return new AckMessage(SrcpMessage::mtAckHandShake, code,
                            tokens[4].toUInt());
                }
            }
            else if (tokens[3] == "PROTOCOL") {
                if (tokens.size() > 4) {
                    return new AckMessage(SrcpMessage::mtAckHandShake, code,
                            tokens[4]);
                }
            }
            else if (tokens[3] == "CONNECTIONMODE") {
                return new AckMessage(SrcpMessage::mtAckHandShake, code);
            }
        }
        // normal operation
        else {
            return new AckMessage(SrcpMessage::mtAckCommand, code);
        }
    }

    /*
     * ERROR
     *
     * <time> 4xx ERROR <message>
     *   0     1    2      3
     */
    else if (tokens[2] == "ERROR") {
        if (tokens.size() > 3) {
            return new AckMessage(SrcpMessage::mtAckCommand, code,
                            line.section(' ', 3));
        }
    }


    /* <time> <code> INFO <bus> <devicegroup> <more...>
     *   0      1     2     3        4          5 */
    else if (tokens[2] == "INFO") {
        unsigned int bus = tokens[3].toUInt();
    /*
     * DESCRIPTION
     *
     * <time> 100 INFO <bus> DESCRIPTION <device list>
     *   0     1   2     3       4            5...
     */
        if (tokens[4] == "DESCRIPTION") {
            int map = 0;
            for (int i = 5; i < tokens.size(); ++i) {
                if (tokens[i] == "FB")
                    map |= SrcpMessage::devFb;
                if (tokens[i] == "GA")
                    map |= SrcpMessage::devGa;
                if (tokens[i] == "GL")
                    map |= SrcpMessage::devGl;
                if (tokens[i] == "GM")
                    map |= SrcpMessage::devGm;
                if (tokens[i] == "LOCK")
                    map |= SrcpMessage::devLock;
                if (tokens[i] == "POWER")
                    map |= SrcpMessage::devPower;
                if (tokens[i] == "SERVER")
                    map |= SrcpMessage::devServer;
                if (tokens[i] == "SM")
                    map |= SrcpMessage::devSm;
                if (tokens[i] == "TIME")
                    map |= SrcpMessage::devTime;
            }
            return new DescriptionMessage(SrcpMessage::mtDescriptionInfo,
                    code, bus, map);
        }

    /*
     * FB
     *
     * <time> 100 INFO <bus> FB <addr> <value>
     *   0     1   2     3   4    5       6
     * <time> 101 INFO <bus> FB
     *   0     1   2     3   4
     * <time> 102 INFO <bus> FB
     *   0     1   2     3   4
     * <time> 103 INFO <bus> FB <addr> 1 GL <addr> <drivemode> <V>
     *   0     1   2     3   4    5    6 7    8         9       10
     */
        else if (tokens[4] == "FB") {
            switch (code) {
                case SrcpMessage::code100:
                    if (tokens.size() > 6) {
                        return new FbMessage(SrcpMessage::mtFbInfo, code,
                                bus, tokens[5].toUInt(), tokens[6].toUInt());
                    }
                    error = tr("Unsupported code: %1").arg(line);
                    break;
                case SrcpMessage::code101:
                case SrcpMessage::code102:
                        return new FbMessage(SrcpMessage::mtFbInfo, code,
                                bus);
                case SrcpMessage::code103:
                    if (tokens.size() > 10) {
                        return new FbMessage(SrcpMessage::mtFbInfo, code,
                                bus, tokens[5].toUInt(),
                                tokens[6].toUInt(),   // address
                                SrcpMessage::getDeviceId(tokens[7]),
                                tokens[8].toUInt(),   // gl-address
                                tokens[9].toUInt(),   // gl-drivemode
                                tokens[10].toUInt()); // gl-velocity
                    }
                    error = tr("Unsupported code: %1").arg(line);
                    break;
                default:
                    break;
            }
        }

    /*
     * GA
     *
     * <time> 100 INFO <bus> GA <addr> <port> <value>
     * <time> 101 INFO <bus> GA <addr> <prot> [<optionales>]
     * <time> 102 INFO <bus> GA <addr>
     *   0     1   2     3   4    5       6      7
     */
        else if (tokens[4] == "GA") {
            unsigned int addr = tokens[5].toUInt();
            switch (code) {
                case SrcpMessage::code100:
                    if (tokens.size() > 7) {
                        return new GaMessage(SrcpMessage::mtGaInfo, code,
                                bus, addr, GaMessage::proNone,
                               tokens[6].toUInt(),  // port
                               tokens[7].toUInt()); // value
                    }
                    error = tr("Unsupported code: %1").arg(line);
                    break;
                 case SrcpMessage::code101:
                    if (tokens.size() > 6) {
                        GaMessage::Protocol prot =
                            GaMessage::getProtocolId(tokens[5]);
                        return new GaMessage(SrcpMessage::mtGaInfo, code,
                                bus, addr, prot);
                    }
                    error = tr("Unsupported code: %1").arg(line);
                    break;
                 case SrcpMessage::code102:
                     return new GaMessage(SrcpMessage::mtGaInfo, code,
                             bus, addr);
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }
        }

    /*
     * GL  responses for 100: SET, 101: INIT, 102: TERM
     *
     * <time> 100 INFO <bus> GL <addr> <drivemode> <v> <vmax> <f1>...<fn>
     * <time> 101 INFO <bus> GL <addr> <prot> <protv> <maxsteps> <fcount>
     * <time> 102 INFO <bus> GL <addr>
     *   0     1   2     3   4    5      6         7     8     9 ...<x>
     */
        else if (tokens[4] == "GL") {

            if (tokens.size() <= 5) {
                error = tr("Unsupported code: %1").arg(line);
                return NULL;
            }
            unsigned int addr = tokens[5].toUInt();
            switch (code) {
                 case SrcpMessage::code100:
                     if (tokens.size() > 9) {
                         // calculate function count by number of given
                         // f-values
                         int fcount = tokens.size() - 9;
                         int fmap = 0;
                         unsigned int bit = 1;
                         for (int i = 0; i < fcount; ++i) {
                             if (tokens[9+i] == "1")
                                 fmap |= bit << i;
                         }
                         return new GlMessage(SrcpMessage::mtGlInfo,
                             code, bus, addr, fcount,
                             tokens[6].toUInt(), //drivemode
                             tokens[7].toUInt(), //v
                             tokens[8].toUInt(), //vmax
                             fmap);              // function map
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code101:
                     if (tokens.size() > 9) {
                         return new GlMessage(SrcpMessage::mtGlInfo,
                                 code, bus, addr,
                                 GlMessage::getProtocolId(tokens[6]),
                                 tokens[7].toUInt(), //protocol version
                                 tokens[8].toUInt(), //decoder steps
                                 tokens[9].toUInt()  //function count
                                 );
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code102:
                     return new GlMessage(SrcpMessage::mtGlInfo,
                             code, bus, addr);
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }

        }

    /*
     * GM
     *
     * <time> 100 INFO <bus=0> GM <sendto> <replyto> <gmtype> <content>
     *   0     1   2     3     4     5         6         7        8
     */
        else if (tokens[4] == "GM") {
            if (tokens.size() > 8) {
                QString content = line.section(' ', 8);
                return new GmMessage(SrcpMessage::mtGmInfo, code,
                       tokens[5].toUInt(), // sendto
                       tokens[6].toUInt(), // replyto
                       tokens[7], content);
            }
            error = tr("Unsupported code: %1").arg(line);
        }

    /*
     * LOCK
     *
     * <time> 100 INFO <bus> LOCK <devicegroup> <addr> <duration> <sessionid>
     * <time> 101 INFO <bus> LOCK <devicegroup> <addr>
     * <time> 102 INFO <bus> LOCK <devicegroup> <addr>
     *   0     1   2     3    4         5        6         7          8
     */
        else if (tokens[4] == "LOCK") {
            if (tokens.size() > 6) {
                SrcpMessage::Device dev = SrcpMessage::getDeviceId(tokens[5]);
                unsigned int addr = tokens[6].toUInt();

                switch (code) {
                    case SrcpMessage::code100:
                        if (tokens.size() > 8) {
                            return new LockMessage(SrcpMessage::mtLockInfo,
                                    code, bus, dev, addr,
                                    tokens[7].toUInt(),  // duration
                                    tokens[8].toUInt()); // sessionid
                        }
                        error = tr("Unsupported code: %1").arg(line);
                        break;
                    case SrcpMessage::code101:
                    case SrcpMessage::code102:
                        return new LockMessage(SrcpMessage::mtLockInfo,
                                code, bus, dev, addr);
                    default:
                        error = tr("Unsupported code: %1").arg(line);
                        break;
                }
            }
            else
                error = tr("Unsupported code: %1").arg(line);
        }

    /*
     * POWER
     *
     * <time> 100 INFO <bus> POWER <on/off>
     * <time> 101 INFO <bus> POWER
     * <time> 102 INFO <bus> POWER
     *   0     1   2     3    4       5
     */
        else if (tokens[4] == "POWER") {
            switch (code) {
                case SrcpMessage::code100:
                    if (tokens.size() > 5) {
                        if (tokens[5] == "ON")
                            return new PowerMessage(
                                    SrcpMessage::mtPowerInfo, code,
                                    bus, true);
                        else if (tokens[5] == "OFF")
                            return new PowerMessage(
                                    SrcpMessage::mtPowerInfo, code,
                                    bus, false);
                        else
                            error = tr("Unsupported code: %1").arg(line);
                    }
                    else
                        error = tr("Unsupported code: %1").arg(line);
                    break;
                case SrcpMessage::code101:
                case SrcpMessage::code102:
                    return new PowerMessage(
                            SrcpMessage::mtPowerInfo, code, bus);
                default:
                    error = tr("Unsupported code: %1").arg(line);
                    break;
            }
        }

    /*
     * SERVER
     *
     * <time> 100 INFO 0 SERVER <status info>
     * <time> 101 INFO 0 SERVER
     * <time> 102 INFO 0 SERVER
     *   0     1   2   3   4       5
     */
        else if (tokens[4] == "SERVER") {
            switch (code) {
                case SrcpMessage::code100:
                    if (tokens.size() > 5) {
                        ServerMessage::ServerState state =
                            ServerMessage::ssNone;
                        if (tokens[5] == "RUNNING")
                            state = ServerMessage::ssRunning;
                        else if (tokens[5] == "RESETTING")
                            state = ServerMessage::ssResetting;
                        else if (tokens[5] == "TERMINATING")
                            state = ServerMessage::ssTerminating;

                        return new ServerMessage(SrcpMessage::mtServerInfo,
                                code, state);
                    }
                    error = tr("Unsupported code: %1").arg(line);
                    break;
                 case SrcpMessage::code101:
                 case SrcpMessage::code102:
                     return new ServerMessage(SrcpMessage::mtServerInfo,
                             code);
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }
        }

    /*
     * SESSION
     *
     * <time> 100 INFO 0 SESSION <id> [parameters]
     * <time> 101 INFO 0 SESSION <id>
     * <time> 102 INFO 0 SESSION <id>
     *   0     1   2   3    4     5         6
     */
        else if (tokens[4] == "SESSION") {
            unsigned int id = tokens[5].toUInt();
            switch (code) {
                 case SrcpMessage::code100:
                     return new SessionMessage(SrcpMessage::mtSessionInfo,
                             code, id);
                 case SrcpMessage::code101:
                 case SrcpMessage::code102:
                     return new SessionMessage(SrcpMessage::mtSessionInfo,
                             code, id);
                     break;
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }
        }

    /*
     * SM
     *
     * <time> 100 INFO <bus> SM <addr> <smtype> <lokUid>
     * <time> 100 INFO <bus> SM <addr> <smtype> <smaddr> <smvalue>
     * <time> 100 INFO <bus> SM <addr> <smtype> <smaddr> <bit> <smvalue>
     * <time> 101 INFO <bus> SM <prot>
     * <time> 102 INFO <bus> SM
     *   0     1   2     3   4    5        6        7      8       9
     */
        else if (tokens[4] == "SM") {
            switch (code) {
                 case SrcpMessage::code100:
                     if (tokens.size() > 7) {
                         unsigned int addr = tokens[5].toUInt();
                         SmMessage::SmType smt =
                             SmMessage::getSmTypeId(tokens[6]);
                         unsigned int smaddr = tokens[7].toUInt();

                         if (tokens.size() > 9) {
                             unsigned int smbit = tokens[8].toUInt();
                             unsigned int smv = tokens[9].toUInt();

                             return new SmMessage(SrcpMessage::mtSmInfo,
                                     code, bus, addr, smt, smaddr,
                                     smbit, smv);
                         }
                         else {
                             unsigned int smv = 0;
							 if (tokens.size() > 8) smv = tokens[8].toUInt();
                             return new SmMessage(SrcpMessage::mtSmInfo,
                                     code, bus, addr, smt, smaddr, smv);
                         }
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code101:
                     if (tokens.size() > 5) {
                         int smp = SmMessage::getSmProtocolId(tokens[5]);
                         return new SmMessage(SrcpMessage::mtSmInfo, code,
                                 bus, smp);
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code102:
                     return new SmMessage(SrcpMessage::mtSmInfo, code,
                             bus);
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }
        }

    /*
     * TIME
     *
     * <time> 100 INFO <bus> TIME <JulDay> <Hour> <Minute> <Second>
     * <time> 101 INFO <bus> TIME <fx>     <fy>
     * <time> 102 INFO <bus> TIME
     *   0     1   2     3    4      5       6       7        8
     */
        else if (tokens[4] == "TIME") {
            switch (code) {
                 case SrcpMessage::code100:
                     if (tokens.size() > 8) {
                         return new TimeMessage(SrcpMessage::mtTimeInfo, code,
                                 tokens[5].toUInt(),  // day
                                 tokens[6].toUInt(),  // hour
                                 tokens[7].toUInt(),  // min
                                 tokens[8].toUInt()); // sec
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code101:
                     if (tokens.size() > 6) {
                         return new TimeMessage(SrcpMessage::mtTimeInfo, code,
                                 tokens[5].toUInt(),  // fx
                                 tokens[6].toUInt()); // fy
                     }
                     error = tr("Unsupported code: %1").arg(line);
                     break;
                 case SrcpMessage::code102:
                     return new TimeMessage(SrcpMessage::mtTimeInfo, code);
                 default:
                     error = tr("Unsupported code: %1").arg(line);
                     break;
            }
        }
        else
            error = QString(tr("Unknown device group: %1")).arg(line);

    }
    else
        error = QString(tr("Unknown response: %1")).arg(line);

    return NULL;
}

