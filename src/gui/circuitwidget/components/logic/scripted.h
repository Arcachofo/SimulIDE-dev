/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef SCRIPTED_H
#define SCRIPTED_H

#include <QScriptEngine>
#include <QScriptProgram>

#include "e-logic_device.h"
#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT Scripted : public LogicComponent, public eLogicDevice
{
    Q_OBJECT
    Q_PROPERTY( quint64 Tpd_ps  READ propDelay   WRITE setPropDelay   DESIGNABLE true USER true )
    Q_PROPERTY( quint64  Tr_ps READ riseTime WRITE setRiseTime DESIGNABLE true USER true )
    Q_PROPERTY( quint64  Tf_ps READ fallTime WRITE setFallTime DESIGNABLE true USER true )
    Q_PROPERTY( double Input_High_V READ inputHighV WRITE setInputHighV DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Low_V  READ inputLowV  WRITE setInputLowV  DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Imped  READ inputImp   WRITE setInputImp   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_High_V   READ outHighV   WRITE setOutHighV   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Low_V    READ outLowV    WRITE setOutLowV    DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Imped    READ outImp     WRITE setOutImp     DESIGNABLE true USER true )
    Q_PROPERTY( bool   Inverted     READ inverted   WRITE setInverted   DESIGNABLE true USER true )
    Q_PROPERTY( int    Num_Inputs   READ numInps    WRITE setNumInps    DESIGNABLE true USER true )
    Q_PROPERTY( int    Num_Outputs  READ numOuts    WRITE setNumOuts    DESIGNABLE true USER true )
    Q_PROPERTY( QString Script      READ getScript  WRITE setScript     DESIGNABLE true USER true )

    public:
        
        Scripted( QObject* parent, QString type, QString id );
        ~Scripted();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

         virtual void stamp() override;
         virtual void voltChanged() override;
         virtual void runEvent() override;

         QString getScript() { return m_script; }
         void setScript( QString f ) { m_script = f; }

         QString evaluate( QString script );
        
        void setNumInps( int inputs );
        void setNumOuts( int outs );
        
    public slots:
        void displayMsg( QString msg ) { qDebug() << msg; }
        bool getInputState( int i ){ return eLogicDevice::getInputState( i ); }
        bool getOutputState( int o ){ return eLogicDevice::getOutputState( o ); }
        void setOutputState( int i, bool s ){ if( i < m_numOutputs ) m_output[i]->setTimedOut(s); }
        int  numInps(){ return m_numInputs; }
        int  numOuts(){ return m_numOutputs; }

    private slots:
        void loadScript();
        void saveScript();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event);

    private:
        QString getExceptions();

        QScriptEngine m_engine;
        QScriptProgram m_program;
        QScriptValue m_thisObject;
        QScriptValue m_voltChanged;

        QString m_script;
        QString m_lastDir;
        QString m_evaluation;

        bool m_canRun;
};

#endif
