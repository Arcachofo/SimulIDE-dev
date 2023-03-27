/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include <QPainter>
#include <QGraphicsProxyWidget>
#include <QToolButton>

#include "ky023.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"

#define WIDTH 40
#define HEIGHT 56
#define GAP 0
#define JOYSTICK_SIZE 36

#define VIN 5

Component* KY023::construct( QObject* parent, QString type, QString id )
{ return new KY023( parent, type, id ); }

LibraryItem* KY023::libraryItem()
{
    return new LibraryItem(
        tr( "KY-023" ),
        "Perifericals",
        "ky-023.png",
        "KY023",
        KY023::construct);
}

KY023::KY023( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area = QRect( -WIDTH/2, -HEIGHT/2 + GAP, WIDTH, HEIGHT );

    m_joystickW.setupWidget();
    m_joystickW.setFixedSize( JOYSTICK_SIZE, JOYSTICK_SIZE );
    /// onvaluechanged(500, 500);
    
    m_proxy = Circuit::self()->addWidget( &m_joystickW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2 + (WIDTH - JOYSTICK_SIZE) / 2, -HEIGHT/2+2 + 2*GAP) );
    
    m_button = new QToolButton();
    m_button->setMaximumSize( 10,10 );
    m_button->setGeometry(-10,-10,10,10);
    
    m_proxy_button = Circuit::self()->addWidget( m_button );
    m_proxy_button->setParentItem( this );
    m_proxy_button->setPos( QPoint(8, HEIGHT/2-13) );

    m_pin.resize(3);

    m_vrx = new IoPin( 270, QPoint(-12,36), id+"-vrx", 0, this, output );
    m_vrx->setOutHighV( VIN );
    m_vrx->setLabelText( "VRX" );
    m_pin[0] = m_vrx;

    m_vry = new IoPin( 270, QPoint(-4,36), id+"-vry", 0, this, output );
    m_vry->setOutHighV( VIN );
    m_vry->setLabelText( "VRY" );
    m_pin[1] = m_vry;

    m_sw = new IoPin( 270, QPoint(4,36), id+"-sw", 0, this, output );
    m_sw->setOutHighV( VIN );
    m_sw->setLabelText( "SW" );
    m_pin[2] = m_sw;

    setAngle( 90 );
    setLabelPos(-34, 20,-90 );
    
    Simulator::self()->addToUpdateList( this );

    connect( m_button, &QToolButton::pressed,
             this,     &KY023::onbuttonpressed);
    
    connect( m_button, &QToolButton::released,
             this,     &KY023::onbuttonreleased);
    
    initialize();
}
KY023::~KY023(){}

void KY023::onbuttonpressed()
{
    m_closed = true;
    m_changed = true;
    
    update();
}

void KY023::onbuttonreleased()
{
    m_closed = false;
    m_changed = true;
    
    update();
}

void KY023::updateStep()
{
    if( m_changed )
    {
        m_changed = false;
        m_sw->setOutState( !m_closed );
    }

    if( !m_joystickW.changed() ) return;

    m_vrx->setOutHighV( VIN*m_joystickW.getXValue()/1000 );
    m_vrx->setOutState( true );

    m_vry->setOutHighV( VIN*m_joystickW.getYValue()/1000 );
    m_vry->setOutState( true );

    m_changed = false;
}

void KY023::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );
}
