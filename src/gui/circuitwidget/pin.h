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

#ifndef PIN_H
#define PIN_H

#include "component.h"
#include "e-pin.h"

enum pinState_t{
    undef_state=0,
    out_open,
    out_low,
    out_high,
    input_low,
    input_high
};

class Connector;

class MAINMODULE_EXPORT Pin : public QObject, public QGraphicsItem, public ePin
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    public:
        Pin( int angle, const QPoint pos, QString id, int index, Component* parent = 0 );
        ~Pin();

        enum { Type = UserType + 3 };
        int type() const { return Type; }

        QRectF boundingRect() const { return m_area; }

        QString pinId() { return m_id; }
        
        bool unused() {return m_unused; }
        void setUnused( bool unused );

        int length() { return m_length; }
        void setLength( int length );

        void setColor( QColor color ) { m_color[0] = color; }
        void setPinAngle( int angle );
        int pinAngle() { return m_angle; }

        void setY( qreal y );

        void setBoundingRect( QRect area ) { m_area = area; }
        
        Component* component() { return m_component; }

        Connector* connector() { return my_connector; }
        void setConnector( Connector* c );
        void removeConnector();

        void setConPin( Pin* pin ) { m_conPin = pin; }
        Pin* conPin(){ return m_conPin; }

        void connectPin();

        QString getLabelText() { return m_labelText; }
        void setLabelPos();
        void setLabelColor( QColor color );
        void setFontSize( int size );
        int  labelSizeX();
        void setLabelText( QString label );

        void setPinId( QString id ) { m_id = id; }

        void setVisible( bool visible );

        void moveBy( int dx, int dy );

        void reset();

        void registerEnode( eNode* enode );
        void registerPinsW( eNode* enode );
        
        void setIsBus( bool bus );
        bool isBus() { return m_isBus; }

        void setPinState( pinState_t st ) { m_pinState = st; m_changed = true; }

        void updateStep() { if( m_changed ) update(); }

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void isMoved();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );

        pinState_t m_pinState;
        bool m_changed;

        int m_angle;
        int m_length;

        bool m_blocked;
        bool m_isBus;
        bool m_unused;

        QString m_labelText;
        
        QColor m_color[6];
        QRect      m_area;
        Connector* my_connector;
        Component* m_component;
        Pin*       m_conPin;          // Pin at the other side of connector

        QGraphicsSimpleTextItem m_label;
};

#endif
