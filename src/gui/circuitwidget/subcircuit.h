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

        virtual void setLogicSymbol( bool ls );

        Component* getMainComp(){ return m_mainComponent; }

        virtual QString toString() override;

    public slots:
        void slotAttach();
        void slotDetach();

    protected:
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

        void loadSubCircuit( QString file );

        Pin* getConPin( QString pinId );

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
};
#endif

