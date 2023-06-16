/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "ledrgb.h"
#include "connector.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "pin.h"

#include "boolprop.h"
#include "doubleprop.h"

Component* LedRgb::construct( QObject* parent, QString type, QString id )
{ return new LedRgb( parent, type, id ); }

LibraryItem* LedRgb::libraryItem()
{
    return new LibraryItem(
        tr( "Led Rgb" ),
        "Leds",
        "ledrgb.png",
        "LedRgb",
        LedRgb::construct);
}

LedRgb::LedRgb( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( id )
{
    m_graphical = true;
    m_color = QColor(0,0,0);
    bright[0] = 15;
    bright[1] = 15;
    bright[2] = 15;

    setComCathode( true );

    m_pin.resize( 4 );
    for( int i=0; i<3; ++i )
    {
        m_pin[i] = new Pin( 180, QPoint(-16, -8+8*i ), m_id+"-Pin"+QString::number(i), 0, this);
        m_pin[i]->setColor( QColor(180*(i==0),120*(i==1),160*(i==2)) );
    }
    m_pin[3] = new Pin( 0, QPoint( 16, 0 ), m_id+"-Pin"+QString::number(3), 0, this);

    for( int i=0; i<3; ++i )
    {
        QString ledId = id+"_Led"+QString::number(i);
        m_led[i] = new eLed( ledId );
        m_led[i]->setEpin( 0, new ePin( ledId+"-ePin"+QString::number(0), 0 ) );
        m_led[i]->setEpin( 1, new ePin( ledId+"-ePin"+QString::number(1), 1 ) );
        if( i > 0 ) m_led[i]->setThreshold( 3.5 );
    }
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Electric"), {
new BoolProp<LedRgb>( "CommonCathode", tr("Common Cathode"),"",  this, &LedRgb::isComCathode, &LedRgb::setComCathode),
new ComProperty( "", tr("Red:"),"","",0),
new DoubProp<LedRgb>( "Threshold_R" , tr("Forward Voltage"),"V", this, &LedRgb::threshold_R,  &LedRgb::setThreshold_R ),
new DoubProp<LedRgb>( "MaxCurrent_R", tr("Max Current")    ,"A", this, &LedRgb::maxCurrent_R, &LedRgb::setMaxCurrent_R ),
new DoubProp<LedRgb>( "Resistance_R", tr("Resistance")     ,"Ω", this, &LedRgb::res_R,        &LedRgb::setRes_R ),
new ComProperty( "", tr("Green:"),"","",0),
new DoubProp<LedRgb>( "Threshold_G" , tr("Forward Voltage"),"V", this, &LedRgb::threshold_G,  &LedRgb::setThreshold_G ),
new DoubProp<LedRgb>( "MaxCurrent_G", tr("Max Current")    ,"A", this, &LedRgb::maxCurrent_G, &LedRgb::setMaxCurrent_G ),
new DoubProp<LedRgb>( "Resistance_G", tr("Resistance")     ,"Ω", this, &LedRgb::res_G,        &LedRgb::setRes_G ),
new ComProperty( "", tr("Blue:"),"","",0),
new DoubProp<LedRgb>( "Threshold_B" , tr("Forward Voltage"),"V", this, &LedRgb::threshold_B,  &LedRgb::setThreshold_B ),
new DoubProp<LedRgb>( "MaxCurrent_B", tr("Max Current")    ,"A", this, &LedRgb::maxCurrent_B, &LedRgb::setMaxCurrent_B ),
new DoubProp<LedRgb>( "Resistance_B", tr("Resistance")     ,"Ω", this, &LedRgb::res_B,        &LedRgb::setRes_B )
    }, groupNoCopy} );
}
LedRgb::~LedRgb(){}

void LedRgb::stamp()
{
    eNode* comEnode = m_pin[3]->getEnode();
    for( int i=0; i<3; ++i )
    {
        eNode* node = m_pin[i]->getEnode();
        eLed* led   = m_led[i];

        if( m_commonCathode ){
            led->getEpin(0)->setEnode( node );
            led->getEpin(1)->setEnode( comEnode );
        }else{
            led->getEpin(1)->setEnode( node );
            led->getEpin(0)->setEnode( comEnode );
}   }   }

void LedRgb::updateStep()
{
    m_warning = false;
    m_crashed = false;
    bool updt = false;

    for( int i=0; i<3; ++i )
    {
        double brg = m_led[i]->brightness()*255+15;
        m_led[i]->updateBright();
        m_warning |= m_led[i]->overCurrent() > 1.5;
        m_crashed |= m_led[i]->overCurrent() > 2;
        bright[i] = m_led[i]->brightness()*255+15;
        if( bright[i] > 255 ) bright[i] = 255;
        updt |= (brg != bright[i]) | m_warning;
    }
    if( updt ) update();
}

void LedRgb::setComCathode( bool cat )
{
    m_commonCathode = cat;
    if( cat ) m_area = QRect(-8, -10, 20, 20 );
    else      m_area = QRect(-12, -10, 20, 20 );

    if( Simulator::self()->isRunning() ) Simulator::self()->stopSim();
    Circuit::self()->update();
}

void LedRgb::setThreshold_R( double threshold )
{ 
    if( threshold < 1e-6 ) threshold = 1e-6;
    m_led[0]->setThreshold( threshold );
}

void LedRgb::setMaxCurrent_R( double current )
{
    if( current < 1e-6 ) current = 1e-6;
    m_led[0]->setMaxCurrent( current );
}

void LedRgb::setRes_R( double resist )
{
    if( resist == 0 ) resist = 1e-14;
    m_led[0]->setRes( resist );
}

void LedRgb::setThreshold_G( double threshold )
{
    if( threshold < 1e-6 ) threshold = 1e-6;
    m_led[1]->setThreshold( threshold );
}

void LedRgb::setMaxCurrent_G( double current )
{
    if( current < 1e-6 ) current = 1e-6;
    m_led[1]->setMaxCurrent( current );
}

void LedRgb::setRes_G( double resist )
{
    if( resist == 0 ) resist = 1e-14;
    m_led[1]->setRes( resist );
}

void LedRgb::setThreshold_B( double threshold )
{
    if( threshold < 1e-6 ) threshold = 1e-6;
    m_led[2]->setThreshold( threshold );
}

void LedRgb::setMaxCurrent_B( double current )
{
    if( current < 1e-6 ) current = 1e-6;
    m_led[2]->setMaxCurrent( current );
}

void LedRgb::setRes_B( double resist )
{
    if( resist == 0 ) resist = 1e-14;
    m_led[2]->setRes( resist );
}

void LedRgb::setHidden( bool hid, bool hidArea, bool hidLabel )
{
    Component::setHidden( hid, hidArea, hidLabel );
    if( hid ) m_area = QRect(-10,-10, 20, 20 );
    else{
        if( m_commonCathode ) m_area = QRect( -8,-10, 20, 20 );
        else                  m_area = QRect(-12,-10, 20, 20 );
    }
}

void LedRgb::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QColor color;

    if( m_warning/*m_current > m_maxCurrent*1.2*/ ) // Led overcurrent
    {
        p->setBrush( QColor( 255, 150, 0 ) );
        color = QColor( Qt::red );
        pen.setColor( color );
    }
    if( m_crashed )  // Led extreme overcurrent
    {
        p->setBrush( Qt::white );
        color = QColor( Qt::white );
        pen.setColor( color );
    }else{
        color = QColor( bright[0], bright[1], bright[2] );
    }
    p->setPen( pen );
    p->drawRoundedRect( m_area, 2, 2 );

    pen.setColor( color );
    pen.setWidth(2.5);
    p->setPen(pen);
    p->setBrush( color );
    p->drawEllipse( m_area );
}
