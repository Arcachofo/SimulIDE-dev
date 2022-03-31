/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef SHIELD_H
#define SHIELD_H

#include "board.h"
//#include "subcircuit.h"

class MAINMODULE_EXPORT ShieldSubc : public BoardSubc
{
    Q_OBJECT

    public:
        ShieldSubc( QObject* parent, QString type, QString id );
        ~ShieldSubc();

        QString boardId() { return m_boardId; }
        void setBoardId( QString id ) { m_boardId = id; }
        void setBoard( BoardSubc* board );

        void connectBoard();

        virtual void remove() override;

    public slots:
        virtual void slotAttach();
        virtual void slotDetach();

    protected:
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        virtual void attachToBoard();
        virtual void renameTunnels();

        bool m_attached; // This is a shield which is attached to a board

        BoardSubc* m_board;  // A board this is attached to (this is a shield)
        QString m_boardId;
        QPointF m_boardPos;
};
#endif

