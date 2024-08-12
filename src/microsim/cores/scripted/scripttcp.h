/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTTCP_H
#define SCRIPTTCP_H

#include "scriptperif.h"
#include "tcpmodule.h"

using namespace std;

class eMcu;
class ScriptCpu;
class asIScriptFunction;

class ScriptTcp : public TcpModule, public ScriptPerif
{
    public:
        ScriptTcp( eMcu* mcu, QString name );
        ~ScriptTcp();

        void reset();

        void connectToHost( int link, const string host, int port );
        void sendMsgToHost( const string msg, int link );

        virtual QStringList registerScript( ScriptCpu* cpu ) override;
        virtual void startScript() override;

        virtual void tcpConnected( int link ) override;
        virtual void tcpDisconnected( int link ) override;
        virtual void received( QString msg, int link ) override;

    private:

        asIScriptFunction* m_received;
        asIScriptFunction* m_connected;
        asIScriptFunction* m_closed;
};

#endif
