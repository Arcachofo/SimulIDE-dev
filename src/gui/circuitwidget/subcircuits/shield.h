/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SHIELD_H
#define SHIELD_H

#include "board.h"

class MAINMODULE_EXPORT ShieldSubc : public BoardSubc
{
    public:
        ShieldSubc( QObject* parent, QString type, QString id );
        ~ShieldSubc();

        QString boardId() { return m_boardId; }
        void setBoardId( QString id ) { m_boardId = id; }
        void setBoard( BoardSubc* board );

        void connectBoard();

        virtual void remove() override;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    public slots:
        virtual void slotAttach();
        virtual void slotDetach();

    protected:
        virtual void attachToBoard();
        virtual void renameTunnels();

        bool m_attached; // This is a shield which is attached to a board

        BoardSubc* m_board;  // A board this is attached to (this is a shield)
        QString m_boardId;
        QPointF m_boardPos;
};
#endif
