/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PIN_H
#define PIN_H

#include "component.h"
#include "updatable.h"
#include "e-pin.h"

enum pinState_t{
    undef_state=0,
    driven_low,    // State is open high, but driven low externally
    open_low,
    open_high,
    out_low,
    out_high,
    input_low,
    input_high
};

class Connector;
class LaChannel;

class MAINMODULE_EXPORT Pin : public QGraphicsItem, public ePin, public Updatable
{
    Q_INTERFACES(QGraphicsItem)

    public:
        Pin( int angle, const QPoint pos, QString id, int index, Component* parent=0 );
        ~Pin();

        enum pinType_t{
            pinNormal=0,
            pinSocket,
            pinHeader,
        };

        enum { Type = UserType + 3 };
        int type() const override { return Type; }

        QRectF boundingRect() const override { return m_area; }

        QString pinId() { return m_id; }
        
        bool unused() {return m_unused; }
        void setUnused( bool unused );

        int length() { return m_length; }
        virtual void setLength( int length );

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

        Pin* connectPin( bool connect );

        QString getLabelText() { return m_labelText; }
        virtual void setLabelText( QString label, bool over=true );
        void setLabelPos();
        void setLabelColor( QColor color );
        void setFontSize( int size );
        int  labelSizeX() { return m_labelWidth; }
        void setSpace( int s );

        void setPinId( QString id ) { m_id = id; }
        void setVisible( bool visible );

        void moveBy( int dx, int dy );

        void remove();

        void registerEnode( eNode* enode, int n=-1 );
        void registerPinsW( eNode* enode, int n=-1 );
        
        void setIsBus( bool bus );
        bool isBus() { return m_isBus; }

        void setPinType( pinType_t ty ) { m_pinType = ty; }
        pinType_t pinType() { return m_pinType; }

        void setDataChannel( LaChannel* ch ) { m_dataCannel = ch; }

        inline void setPinState( pinState_t st ) { m_pinState = st; /*m_PinChanged = true;*/ }

        void warning( bool w );
        void animate( bool an );
        virtual void updateStep() override;

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

        void isMoved();
        void flip( int h, int v );

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

        pinType_t  m_pinType;
        pinState_t m_pinState;

        int m_angle;
        int m_length;
        int m_Hflip;
        int m_Vflip;
        int m_overScore;
        int m_labelheight;
        int m_labelWidth;
        int m_space;

        bool m_blocked;
        bool m_isBus;
        bool m_unused;
        bool m_animate;
        bool m_warning;

        double  m_opCount;

        QString m_labelText;
        
        QColor m_color[8];
        QRectF     m_area;
        Connector* my_connector;
        Component* m_component;
        LaChannel* m_dataCannel;    // connect to Logic Analyzer
        Pin*       m_conPin;          // Pin at the other side of connector

        QGraphicsSimpleTextItem m_label;
};

#endif
