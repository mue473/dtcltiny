/***************************************************************************
 srcpport.h
 ----------
 Begin        : 17.08.2007
 Last modified: $Date: 2007/09/04 19:27:47 $
                $Revision: 1.3 $
 Copyright    : (C) 2007 by Guido Scholz <guido-scholz@gmx.net>
 Description  : Abstract class for network communication with SRCP server.
                Only communication style SRCP 0.8 is supported.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SRCPPORT_H
#define SRCPPORT_H

#include <QString>
#include <QTcpSocket>

#include "srcpmessagefactory.h"


class SrcpPort: public QObject {
  Q_OBJECT

    QString host;
    unsigned int port;
    bool translateservertime;
    QString otherProtocol;
    bool reconnect;
    unsigned int sessionid;
    QString srcpVersion;
    QString srcpOther;
    QString srcpServer;
    QTcpSocket* srcpSocket;

    void clearConnectionData();
    QString getSocketErrorString(QAbstractSocket::SocketError);
    QString translateServerTime(const QString&);
    SrcpMessageFactory smfactory;

   
public:
    SrcpPort(QObject* parent = 0, const char * name = 0,
            const char* host = "localhost", unsigned int port = 4303,
            bool translate = false);

    void setServer(const QString&, unsigned int);
    void setPreferedProtocol(const QString&);
    QString getHostname() const;
    unsigned int getPortNumber() const;
    bool hasServerConnection();
    void serverConnect();
    void serverDisconnect();
    void sendToServer(const QString&);
    unsigned int getSessionId();
    QString getSrcpVersion();
    QString getSrcpOther();
    QString getSrcpServer();
    void enableTimeTranslation(bool);
    bool isTimeTranslationEnabled();

protected:
    enum SrcpState{sNone, sLogin, sProtocol, sConnectionMode, sGo, sRun};
    SrcpState srcpState;
    virtual void setPresetState();
    virtual QString getConnectionMode() = 0;
    
private slots:
    void readData();
    void hostFound();
    void socketConnected();
    void socketClosed();
    void socketDelayedClosed();
    void socketError(QAbstractSocket::SocketError);

signals:
    void connectionStateChanged(bool);
    void messageReceived(const QString&);
    void messageReceived(const SrcpMessage*);
    void messageSend(const QString&);
    void statusMessage(const QString&);
};

#endif   //SRCPPORT_H
