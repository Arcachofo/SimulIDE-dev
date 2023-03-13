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

        QPointF getLabelPos() { return pos(); }
        void setLabelPos( QPointF pos );
        void updtLabelPos();

        int  getAngle()            { return m_labelrot; }
        void setAngle( int angle ) { m_labelrot = angle;  }

        QString itemType() { return "Label"; }

    public slots:
        void rotateCW();
        void rotateCCW();
        void rotate180();
        void H_flip( int hf );
        void V_flip( int vf );
        void updateGeometry(int, int, int);

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

    private:
        Component* m_parentComp;

        int m_labelx;
        int m_labely;
        int m_labelrot;
};
#endif
