/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include "compbase.h"
#include "updatable.h"

class Pin;
class Label;

class MAINMODULE_EXPORT Component : public CompBase, public QGraphicsItem, public Updatable
{
    Q_OBJECT
    Q_INTERFACES( QGraphicsItem )

    public:
        QRectF boundingRect() const { return QRectF( m_area.x()-2, m_area.y()-2, m_area.width()+4, m_area.height()+4 ); }

        Component( QObject* parent, QString type, QString id );
        ~Component();

        virtual bool setProperty( QString prop, QString val ) override;

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QPointF position() { return pos(); }
        void setPosition( QPointF pos ) { setPos( pos ); }

        double getAngle() { return rotation(); }
        void setAngle( double angle ) { setRotation( angle ); }

        QString idLabel();
        void setIdLabel( QString id );

        void setLabelPos( int x, int y, int rot=0 );
        void updtLabelPos();

        bool showId() { return m_showId; }
        void setShowId( bool show );

        QString showProp() { return m_showProperty; }
        void setShowProp( QString prop );

        void setValLabelText( QString t );
        void setValLabelPos( int x, int y, int rot );
        void updtValLabelPos();

        int  hflip() { return m_Hflip; }
        virtual void setHflip( int hf );

        int  vflip() { return m_Vflip; }
        void setVflip( int vf );

        virtual void setHidden( bool hid , bool hidLabel=false );

        virtual void setBackground( QString bck ) { m_background = bck;}
        virtual void setSubcDir( QString dir ) {;}

        QString print();

        virtual void inStateChanged( int ){;}

        virtual void move( QPointF delta ) { setPos( pos() + delta ); emit moved(); }
        void moveTo( QPointF pos ){ setPos( pos ); emit moved(); }

        virtual void remove();

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* );

        Component* m_subcircuit;

        bool m_printable;

    signals:
        void moved();

    public slots:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected slots:
        void slotProperties();
        void rotateCW();
        void rotateCCW();
        void rotateHalf();
        void H_flip();
        void V_flip();
        void slotRemove();
        void slotCopy();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
            { if( event->button() == Qt::LeftButton ) slotProperties(); }

        void setflip();

        double  m_opCount;
        
        int m_Hflip;
        int m_Vflip;
 static int m_error;
 static bool m_selMainCo;

        Label* m_idLabel;
        Label* m_valLabel;

        QString m_help;
        QString m_background;   // BackGround Image path

        QColor  m_color;
        QRectF  m_area;         // bounding rect
        QPointF m_eventpoint;

        std::vector<Pin*> m_pin;
};

typedef Component* (*createItemPtr)( QObject* parent, QString type, QString id );

#endif
