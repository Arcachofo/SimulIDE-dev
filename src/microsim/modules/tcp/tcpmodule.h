/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TCPMODULE_H
#define TCPMODULE_H

#include <QMap>

#include "e-element.h"
#include "updatable.h"

class QTcpSocket;
class QSignalMapper;

class TcpModule : public eElement, public Updatable
{
    public:
        TcpModule( QString name );
        ~TcpModule();

        enum tcpAction_t{
            tcpNone =0,
            tcpConnect,
            tcpClose,
        };

        struct tcpConnection_t{
            QTcpSocket* socket;
            QString     host;
            int         port;
            int         number;
            tcpAction_t action;
            QByteArray  toSend;
        };

        virtual void initialize() override;
        virtual void updateStep() override;

        void connectTo( int link, QString host, int port );
        void sendMsg( QString msg, int link );
        void closeSocket( int link );

        bool debug() { return m_debug; }
        void setDebug( bool d) { m_debug = d; }

        virtual void tcpConnected( int link );
        virtual void tcpDisconnected( int link );
        virtual void received( QString msg, int link ){;}

    protected:
        void tcpReadyRead( int link );

        bool m_debug;

        QMap<int, tcpConnection_t*> m_tcpConnections;

        QSignalMapper* m_connectSM;
        QSignalMapper* m_discontSM;
        QSignalMapper* m_readyReSM;
};

#endif
