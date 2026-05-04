/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include <QPainter>
#include <QGraphicsProxyWidget>

#include "ky023.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "custombutton.h"
#include "itemlibrary.h"

#define WIDTH  40 // 5*8
#define HEIGHT 56 // 7*8
#define JOYSTICK_SIZE 50
#define VIN 5

#define tr(str) simulideTr("KY023",str)

Component* KY023::construct( QString type, QString id )
{ return new KY023( type, id ); }

LibraryItem* KY023::libraryItem()
{
    return new LibraryItem(
        tr("Joystick Dual Axis"),
        "Peripherals",
        "ky-023.png",
        "KY023",
        KY023::construct);
}

KY023::KY023( QString type, QString id )
     : Component( type, id )
     , eElement( id )
{
    m_graphical = true;

    m_area = QRect( -WIDTH/2, -HEIGHT/2, WIDTH, HEIGHT );

    m_joystickW.setupWidget( JOYSTICK_SIZE );

    m_proxy = Circuit::self()->addWidget( &m_joystickW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-JOYSTICK_SIZE/2,-JOYSTICK_SIZE/2-8 ) );
    
    m_button = new CustomButton();
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
    m_sw->setOutputImp( 2000 );
    m_sw->setLabelText( "SW" );
    m_pin[2] = m_sw;

    //setAngle( 90 );
    setLabelPos(-34, 20,-90 );

    Simulator::self()->addToUpdateList( this );

    QObject::connect( m_button, &QToolButton::pressed , [=](){ onbuttonpressed(); });
    QObject::connect( m_button, &QToolButton::released, [=](){ onbuttonreleased(); });

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
        m_sw->setOutputImp( m_closed ? 1e-3 : 2000 );
        m_sw->setOutState( !m_closed );
    }

    if( !m_joystickW.changed() ) return;

    QPointF pos = m_joystickW.getPos();

    m_vrx->setOutHighV( VIN*pos.x()/1000 );
    m_vrx->setOutState( true );

    m_vry->setOutHighV( VIN*pos.y()/1000 );
    m_vry->setOutState( true );

    m_changed = false;
}

void KY023::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );
    
    p->setBrush( QColor( 58, 60, 52 ) );
    p->drawRoundedRect( m_area, 2, 2 );

    QFont font = p->font();
    font.setPixelSize( 5 );
    p->setFont( font );
    p->setPen( QColor( 200, 200, 200 ) );
    p->drawText( QPointF( 2-WIDTH/2, 6-HEIGHT/2 ),"0");
    p->drawText( QPointF( WIDTH/2-6, 6-HEIGHT/2 ),"X");
    p->drawText( QPointF( 2-WIDTH/2, WIDTH/2-10 ),"Y");

    Component::paintSelected( p );
}
