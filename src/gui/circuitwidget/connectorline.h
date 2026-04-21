/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
 
#pragma once

#include <QGraphicsItem>

class Connector;
 
class ConnectorLine : public QGraphicsItem
{
    Q_INTERFACES(QGraphicsItem)

    friend class Connector;

    public:
        ConnectorLine( int x1, int y1, int x2, int y2, Connector* connector );
        ~ConnectorLine();

        enum { Type = UserType + 2 };
        int type() const { return Type; }

        virtual QRectF boundingRect() const;

        void setConnector( Connector* con ) { m_pConnector = con; }
        Connector* connector() { return m_pConnector; }
        
        void setPrevLine( ConnectorLine* prevLine ) { m_prevLine = prevLine; }
        void setNextLine( ConnectorLine* nextLine ) { m_nextLine = nextLine; }

        void setP1( QPoint p ) { if( m_prevLine ) m_prevLine->sSetP2( p ); sSetP1( p ); }
        void setP2( QPoint p ) { if( m_nextLine ) m_nextLine->sSetP1( p ); sSetP2( p ); }

        QPoint p1() { return QPoint( m_p1X, m_p1Y ); }
        QPoint p2() { return QPoint( m_p2X, m_p2Y ); }

        int dx() { return (m_p2X - m_p1X);}
        int dy() { return (m_p2Y - m_p1Y);}
        
        bool isDiagonal();

        void move( QPointF delta );
        void moveLine( QPoint delta );
        void moveSimple( QPointF delta );

        void updatePos() { setPos( m_p1X, m_p1Y ); update(); }
        void updateLines() { updatePrev(); updateNext(); }
        void updatePrev() { if( m_prevLine ) m_prevLine->sSetP2( QPoint( m_p1X, m_p1Y) ); }
        void updateNext();

        bool connectToWire( QPoint point1 );
        
        void setIsBus( bool bus ) { m_isBus = bus; }

        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void sSetP1( QPoint );
        void sSetP2( QPoint );
        void remove();

    protected:
        void hoverMoveEvent( QGraphicsSceneHoverEvent* event ) override;
        void hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) override;

    private:
        void updtLength();
        int myIndex();

        int m_p1X;
        int m_p1Y;
        int m_p2X;
        int m_p2Y;

        double m_length; // Length in grid cells

        bool m_animateCurrent;

        QPoint m_mousePos;
        
        bool m_isBus;
        bool m_moveP1;
        bool m_moveP2;
        bool m_moving;

        uint32_t* m_foreColor;

        Connector*     m_pConnector;
        ConnectorLine* m_prevLine;
        ConnectorLine* m_nextLine;
};
