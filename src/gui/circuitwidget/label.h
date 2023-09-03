/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPLABEL_H
#define COMPLABEL_H

#include <QGraphicsTextItem>

class Component;

class Label : public QGraphicsTextItem
{
    friend class Component;

    public:
        Label();
        ~Label();

        void setComponent( Component* parent );

        QPointF getLabelPos() { return QPointF( m_labelx, m_labely ); }
        void setLabelPos( QPointF pos );
        void updtLabelPos();

        int  getAngle()            { return m_labelrot; }
        void setAngle( int angle ) { m_labelrot = angle;  }

        QString itemType() { return "Label"; }

    public slots:
        void rotateCW();
        void rotateCCW();
        void rotate180();
        void updateGeometry(int, int, int);

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        int hFlip();
        int vFlip();

    private:
        Component* m_parentComp;

        float m_labelx;
        float m_labely;
        int m_labelrot;
};
#endif
