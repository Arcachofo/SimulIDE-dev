/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SHIELD_H
#define SHIELD_H

#include "board.h"

class ShieldSubc : public BoardSubc
{
    public:
        ShieldSubc( QString type, QString id );
        ~ShieldSubc();

        QString boardId() { return m_boardId; }
        void setBoardId( QString id ) { m_boardId = id; }

        void setBoard( BoardSubc* board );

        void connectBoard();

        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

        virtual void slotAttach();
        virtual void slotDetach();

    protected:
        virtual void attachToBoard();
        virtual void renameTunnels();

        QString m_boardId;
        QPointF m_boardPos;
};
#endif
