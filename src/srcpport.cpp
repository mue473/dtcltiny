// srcpport.cpp - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
 srcpport.cpp
 ------------
 Begin        : 17.08.2007
 Last modified: $Date: 2008/11/05 08:42:40 $
                $Revision: 1.8 $
 Copyright    : (C) 2007-2008 by Guido Scholz <guido-scholz@gmx.net>
 Description  : Abstract class for network communication with SRCP server.
                Only communication style 0.8 is supported.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "srcpport.h"

#include <QApplication>
#include <QStringList>
#include <QDateTime>


SrcpPort::SrcpPort(QObject* parent, const char* name, const char* hostname,
        unsigned int portnumber, bool translate): QObject(parent),
    host(hostname), port(portnumber), translateservertime(translate),
    otherProtocol(""), reconnect(false), sessionid(0), srcpVersion(""),
    srcpOther(""), srcpServer(""), srcpState(sNone)
{
    setObjectName(name);
    srcpSocket = new QTcpSocket(this);
    Q_CHECK_PTR(srcpSocket);
    srcpSocket->setObjectName("srcpSocket");

    connect(srcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(srcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()));
    connect(srcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(srcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    connect(srcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
#else
    connect(srcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
#endif
            this, SLOT(socketError(QAbstractSocket::SocketError)));
}

/*
 * clear connection data
 */
void SrcpPort::clearConnectionData()
{
    sessionid = 0;
    srcpVersion = "";
    srcpOther = "";
    srcpServer = "";
    srcpState = sNone;
}

/*
 * Set hostname and portnumber to new values, close current connection
 * and reconnect if values change.
 */
void SrcpPort::setServer(const QString& hn, unsigned int prt)
{
    if ((host != hn) || (port != prt)) {

        host = hn;
        port = prt;

        if (hasServerConnection()) {
            reconnect = true;
            serverDisconnect();
        }
    }
}

/*
 * return hostname
 */
QString SrcpPort::getHostname() const
{
    return host;
}

/*
 * return port number
 */
unsigned int SrcpPort::getPortNumber() const
{
    return port;
}

/*
 * enable/disable translation of SRCP 0.8 server time value
 */
void SrcpPort::enableTimeTranslation(bool translate)
{
    translateservertime = translate;
}

/*
 * read if translation of SRCP 0.8 server time value is enabled
 */
bool SrcpPort::isTimeTranslationEnabled()
{
    return translateservertime;
}

/*
 * set other protocol for SRCP 0.8 SRCPOTHER value
 */
void SrcpPort::setPreferedProtocol(const QString& protocol)
{
    otherProtocol = protocol;
}

/*
 * return presetted srcp state; SRCP 0.7 info port jumps directly to run
 * state
 */
void SrcpPort::setPresetState()
{
    srcpState = sLogin;
}

/*
 * initiate server connection and start handshake phase
 */
void SrcpPort::serverConnect()
{
    emit statusMessage(tr("%1: Try to connect host '%2' on port '%3'")
            .arg(getConnectionMode()).arg(host).arg(port));

    setPresetState();
    srcpSocket->connectToHost(host, port);
}

/*
 * stop server connection and reset communication state
 */
void SrcpPort::serverDisconnect()
{
    if (srcpSocket->isOpen()) {
        clearConnectionData();
        srcpSocket->close();
        /*if (srcpSocket->state() == QAbstractSocket::ClosingState) {
            // We have a delayed close.
            connect(srcpSocket, SIGNAL(delayedCloseFinished()),
                    SLOT(socketDelayedClosed()));
        }
        // Socket immediately closed
        else {*/
            clearConnectionData();
            emit connectionStateChanged(false);
            emit statusMessage(tr("%1: Socket immediately closed.")
                    .arg(getConnectionMode()));

            if (reconnect) {
                reconnect = false;
                serverConnect();
            }
        //}
    }
}


/*
 * report current connection state
 */
bool SrcpPort::hasServerConnection()
{
    return (srcpSocket->state() == QAbstractSocket::ConnectedState);
}


/*
 * Response to incomming SRCP messages. In old style mode there are only
 * two states (1. and 5.). In new style mode there is a handshake phase
 * with additional three or four states:
 *
 *  0. sNone: No connection
 *  1. sLogin: Start login and receive welcome message
 *  2. sProtocol: optional, select other protocol version
 *  3. sConnectionMode: select connection mode COMMAND or INFO
 *  4. sGo: Receive session number
 *  5. sRun: Receive SRCP messages and pass them through
 *
*/
void SrcpPort::readData()
{
    while (srcpSocket->canReadLine()) {
        QString line = srcpSocket->readLine();

        // cut off line break
        if (line.endsWith("\n"))
            line.truncate(line.length() - 1);

        // cut off line feed (not implemented yet)

        // normal operation mode, comes first to be faster
        // incoming messages are send als command messages
        if (srcpState == sRun) {
            //qWarning("Info: %s\n", line.data());
            if (translateservertime)
                line = translateServerTime(line);
            emit messageReceived(line);

            SrcpMessage* sm = smfactory.parse(line);
            if (NULL == sm) {
                QString error = smfactory.getErrorStr();
                emit statusMessage(error);
            }
            else {
                emit messageReceived(sm);
                delete sm;
            }

        }

        // handshake, only initial phase of a new connection
        // incoming messages are send als status messages
        else {
            //qWarning("Login: %s\n", line.data());
            if (translateservertime)
                line = translateServerTime(line);
            emit statusMessage(line);
            QStringList tokens;
            QStringList wmt;
            QStringList::Iterator it;
            QString mode;

            // select login state
            switch(srcpState) {
                case sLogin:

                    // get welcome message and split to tokens
                    // we expect a list of three elements
                    tokens = line.split(';');
                    it = tokens.begin();

                    // split tokens to get SRCP, SRCPOTHER
                    // and server values
                    while (it != tokens.end()) {
                        (*it) = QString(*it).trimmed();

                        // here we expect a list of exactly
                        // two elements
                        wmt = (*it).split(' ');

                        if (wmt.count() == 2) {

                            if (wmt[0].toLower() == "srcp")
                                srcpVersion = wmt[1];
                            else if (wmt[0].toLower() == "srcpother")
                                srcpOther = wmt[1];
                            else {
                                if (!srcpServer.isEmpty()) {
                                    srcpServer.append(":");
                                    srcpServer.append(QString(*it));
                                }
                                else
                                    srcpServer = QString(*it);
                            }
                        }
                        else if (wmt.count() > 2)
                            emit statusMessage(
                                tr("%1: Parse error, parameter list too "
                                    "long '%2'.")
                                .arg(getConnectionMode()).arg(QString(*it)));
                        else
                            emit statusMessage(
                                tr("%1: Parse error, parameter list too "
                                    "short '%2'.")
                                .arg(getConnectionMode()).arg(QString(*it)));

                        wmt.clear();
                        ++it;
                    }

                    // check for valid SRCP version
                    // a) No SRCP version string found
                    if (srcpVersion.isEmpty()) {
                        emit statusMessage(
                                tr("%1: Communication error, no SRCP "
                                    "version found.").arg(getConnectionMode()));
                        serverDisconnect();
                    }

                    // c) SRCP 0.8 style connection
                    else if ((srcpVersion >= "0.8.0") &&
                            (srcpVersion < "0.9.0")) {
                        // option to select protocol is only used,
                        // if string value for protocol is given
                        if (!otherProtocol.isEmpty()) {
                            // set next mode
                            srcpState = sProtocol;
                            sendToServer(QString("SET PROTOCOL SRCP %1")
                                    .arg(otherProtocol));
                        }
                        else {
                            // set next mode
                            srcpState = sConnectionMode;
                            mode = getConnectionMode();
                            sendToServer(
                                    QString("SET CONNECTIONMODE SRCP %1")
                                    .arg(mode));
                        }
                    }

                    // d) No valid SRCP version string
                    else {
                        emit statusMessage(
                                tr("%1: Communication error, wrong SRCP "
                                    "version '%2'.")
                                .arg(getConnectionMode()).arg(srcpVersion));
                        serverDisconnect();
                    }

                    break;

                case sProtocol:
                    // FIXME: use message factory
                    // set protocol version
                    // <time> 201 OK PROTOCOL
                    //   0     1  2     3
                    tokens = line.split(' ');
                    if ((tokens.count() < 4) || (tokens[2] != "OK")) {
                        emit statusMessage(tr("%1: Communication error "
                                    "PROTOCOL '%2'")
                                .arg(getConnectionMode()).arg(line));
                        serverDisconnect();
                    }
                    else {
                        // set next mode
                        srcpState = sConnectionMode;
                        mode = getConnectionMode();
                        sendToServer(QString("SET CONNECTIONMODE SRCP %1")
                                .arg(mode));
                    }
                    break;

                case sConnectionMode:
                    // set connection mode
                    // <time> 202 OK CONNECTIONMODE
                    //   0     1  2        3
                    tokens = line.split(' ');
                    if ((tokens.count() < 4) || (tokens[2] != "OK")) {
                        emit statusMessage(tr("%1: Communication error "
                                    "CONNECTIONMODE '%2'")
                                .arg(getConnectionMode()).arg(line));
                        serverDisconnect();
                    }
                    else {
                        // set next mode
                        srcpState = sGo;
                        sendToServer("GO");
                    }
                    break;

                case sGo:
                    {
                    // <time> 200 OK GO <session-id>
                    //   0     1  2  3      4
                    // set next mode
                    SrcpMessage* sm = smfactory.parse(line);
                    if (NULL == sm) {
                        QString error = smfactory.getErrorStr();
                        emit statusMessage(error);
                        serverDisconnect();
                    }
                    else {
                        srcpState = sRun;
                        if (SrcpMessage::mtAckHandShake == sm->getType())
                            sessionid = ((AckMessage*)sm)->getSessionId();
                        emit connectionStateChanged(true);
                        emit messageReceived(sm);
                        delete sm;
                    }
                    }
                    break;

                default:
                    // should never be reached
                    emit statusMessage(
                            tr("%1: Error, wrong SRCP state: %2")
                            .arg(getConnectionMode()).arg(srcpState));
                    break;
            }
        }
    }
}

/*
 * give response if a new socket connection is successfully established
 * for SRCP 07. info port and feedback port connection state change is
 * reported immediately (without login procedure)
 */
void SrcpPort::socketConnected()
{
    emit statusMessage(tr("%1: Socket connected to host '%2' on "
                "port '%3'").arg(getConnectionMode()).arg(host).arg(port));

    if (srcpState == sRun)
        emit connectionStateChanged(true);
}

/*
 * give response if host was found
 */
void SrcpPort::hostFound()
{
    emit statusMessage(tr("%1: Host '%2' found")
            .arg(getConnectionMode()).arg(host));
}

/*
 * socket was closed by peer
 */
void SrcpPort::socketClosed()
{
    if (srcpSocket->isOpen())
        srcpSocket->close();

    clearConnectionData();
    emit connectionStateChanged(false);
    emit statusMessage(tr("%1: Socket closed by foreign host.")
            .arg(getConnectionMode()));
}

/*
 * socket was actively closed;
 * reconnect if server name or port was changed
 */
void SrcpPort::socketDelayedClosed()
{
    clearConnectionData();
    emit connectionStateChanged(false);
    emit statusMessage(tr("%1: Socket delayed closed.")
            .arg(getConnectionMode()));

    if (reconnect) {
        reconnect = false;
        serverConnect();
    }
}

/*
 * show error message text
 */
void SrcpPort::socketError(QAbstractSocket::SocketError error)
{
    QString errorMsg = getSocketErrorString(error);
    emit statusMessage(tr("%1: Socket error %2 occured (%3).")
            .arg(getConnectionMode()).arg(error).arg(errorMsg));
}

/*
 * send SRCP command to server, for handshake phase only
 */
void SrcpPort::sendToServer(const QString& cs)
{
    if (srcpSocket->isOpen()) {
        QByteArray cmd = cs.toLatin1();

        // check for line end
        if (!cs.endsWith("\n"))
            cmd.append("\n");

        srcpSocket->write(cmd);

        // give application some time to send message
        qApp->processEvents();

        if (srcpState == sRun)
            emit messageSend(cs);
        else
            emit statusMessage(cs);
    }
}

/*
 * get session id of current srcp connection
 */
unsigned int SrcpPort::getSessionId()
{
    return sessionid;
}

/*
 * get SRCP version of current srcp session
 */
QString SrcpPort::getSrcpVersion()
{
    return srcpVersion;
}

/*
 * get SRCPOTHER value of current connected SRCP server
 */
QString SrcpPort::getSrcpOther()
{
    return srcpOther;
}

/*
 * get server name and version of current connected SRCP server
 */
QString SrcpPort::getSrcpServer()
{
    return srcpServer;
}

/*
 * return a message string for every numerical error
 */
QString SrcpPort::getSocketErrorString(QAbstractSocket::SocketError e)
{
    QString ErrMessage = "";

    switch (e) {
        case (QAbstractSocket::ConnectionRefusedError):
            ErrMessage = tr("connection refused");
            break;
        case (QAbstractSocket::HostNotFoundError):
            ErrMessage = tr("host not found");
            break;
        case (QAbstractSocket::SocketAccessError):
            ErrMessage = tr("socket read error");
            break;
        default:
            ErrMessage = tr("unknown socket error");
            break;
    }
    return ErrMessage;
}

/*
 * translate SRCP 0.8 server time value to a readable string
 */
QString SrcpPort::translateServerTime(const QString& msg)
{
    QString msgstr;
    QString timestr = msg.section(' ', 0, 0);
    if (!timestr.startsWith("0.")) {
        QDateTime srvtime = QDateTime();
        srvtime.setSecsSinceEpoch(timestr.section('.', 0 , 0).toUInt());
        QTime msgtime = srvtime.time();
        msgstr = msgtime.toString("[hh:mm:ss.");

        msgstr.append(timestr.section('.', 1 , 1));
        msgstr.append("] ");
    }
    else
        msgstr = "[--:--:--.---] ";

    msgstr.append(msg.section(' ', 1));
    return msgstr;
}
