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

#include "logiccomponent.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Scripted : public LogicComponent, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool   Inverted     READ invertOuts WRITE setInvertOuts DESIGNABLE true USER true )
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

    public slots:
        void displayMsg( QString msg ) { qDebug() << msg; }
        bool getInputState( int i ){ return m_inPin[i]->getInpState(); }
        bool getOutputState( int o ){ return m_outPin[o]->getOutState(); }
        void setOutputState( int i, bool s ){ if( (uint)i < m_outPin.size() ) m_outPin[i]->setOutState( s, true ); }
        int  numInps(){ return m_inPin.size(); }
        int  numOuts(){ return m_outPin.size(); }

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
