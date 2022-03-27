/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "chip.h"

class Tunnel;
class QDomDocument;
class LibraryItem;

class MAINMODULE_EXPORT SubCircuit : public Chip
{
    Q_OBJECT

    public:
        SubCircuit( QObject* parent, QString type, QString id );
        ~SubCircuit();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        QString boardId() { return m_boardId; }
        void setBoardId( QString id ) { m_boardId = id; }
        void setBoard( SubCircuit* board ) { m_board = board;  }

        void connectBoard();
        void setShield( SubCircuit* shield ) { m_shield = shield; }

        virtual void remove() override;

        virtual void setLogicSymbol( bool ls ) override;

        Component* getMainComp(){ return m_mainComponent; }

        virtual QString toString() override;

        // Logic:
        double inputHighV() { return m_inHighV; }
        void setInputHighV( double volt )
        {
            m_inHighV = volt;
            for( Component* c : m_compList)
                c->setPropStr("Input_High_V", QString::number(volt)+" V");
        }
        double inputLowV() { return m_inLowV; }
        void setInputLowV( double volt )
        {
            m_inLowV = volt;
            for( Component* c : m_compList)
                c->setPropStr("Input_Low_V", QString::number(volt)+" V");
        }
        double outHighV() { return m_ouHighV; }
        void  setOutHighV( double volt )
        {
            m_ouHighV = volt;
            for( Component* c : m_compList)
                c->setPropStr("Out_High_V", QString::number(volt)+" V");
        }
        double outLowV() { return m_ouLowV; }
        void  setOutLowV( double volt )
        {
            m_ouLowV = volt;
            for( Component* c : m_compList)
                c->setPropStr("Out_Low_V", QString::number(volt)+" V");
        }
        double inputImp() { return m_inImp; }
        void setInputImp( double imp )
        {
            m_inImp = imp;
            for( Component* c : m_compList)
                c->setPropStr("Input_Imped", QString::number(imp)+" Ω");
        }
        double outImp() { return m_ouImp; }
        void  setOutImp( double imp )
        {
            m_ouImp = imp;
            for( Component* c : m_compList)
                c->setPropStr("Out_Imped", QString::number(imp)+" Ω");
        }
        /*double propDelay() { return m_propDelay*1e-12; }
        void setPropDelay( double pd ) { m_propDelay = pd*1e12; }
        double riseTime() { return m_timeLH*1e-12; }
        void setRiseTime( double time );
        double fallTime() { return m_timeHL*1e-12; }
        void setFallTime( double time );*/

    public slots:
        void slotAttach();
        void slotDetach();

    protected:
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

        void loadSubCircuit( QString file );

        virtual void addPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8 );

        virtual void updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle, int length=8  );

        bool m_attached; // This is a shield which is attached to a board

        Component*  m_mainComponent;
        SubCircuit* m_shield; // A shield attached to this
        SubCircuit* m_board;  // A board this is attached to (this is a shield)
        QString     m_boardId;

        QList<Component*> m_compList;
        QHash<QString, Tunnel*> m_pinTunnels;
        QList<Tunnel*> m_subcTunnels;

        double m_inHighV;
        double m_inLowV;
        double m_ouHighV;
        double m_ouLowV;

        double m_inImp;
        double m_ouImp;

        //uint64_t m_propDelay; // Propagation delay
        //uint64_t m_timeLH;    // Time for Output voltage to switch from 10% to 90%
        //uint64_t m_timeHL;    // Time for Output voltage to switch from 90% to 10%
};
#endif

