/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "battery.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "pin.h"

#include "doubleprop.h"

#define tr(str) simulideTr("Battery",str)

Component* Battery::construct( QString type, QString id )
{ return new Battery( type, id ); }

LibraryItem* Battery::libraryItem()
{
    return new LibraryItem(
        tr("Battery"),
        "Sources",
        "battery.png",
        "Battery",
        Battery::construct );
}

Battery::Battery( QString type, QString id )
       : Comp2Pin( type, id )
       , eResistor( id )
{
    m_area = QRect( -10, -10, 20, 20 );

    m_volt = 5;
    m_admit = 1e3;

    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];

    m_pin[0]->setColor( Qt::red );

    setLabelPos(-18,-22, 0 );
    setValLabelPos(-10, 10, 0 ); // x, y, rot

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new DoubProp<Battery>("Voltage", tr("Voltage"),"V"
                             , this, &Battery::voltage, &Battery::setVoltage ),

        new DoubProp<Battery>("Resistance", tr("Resistance"),"mΩ"
                             , this, &Battery::resistance,  &Battery::setResistance ),
    },0 } );
}
Battery::~Battery() {}

void Battery::stamp()
{
    eResistor::stamp();

    m_connected = m_pin[0]->isConnected() && m_pin[1]->isConnected();

    if( m_connected ){
        m_pin[0]->createCurrent();
        m_pin[1]->createCurrent();
        m_pin[0]->stampCurrent( m_volt*m_admit );
        m_pin[1]->stampCurrent(-m_volt*m_admit );
    }
}

void Battery::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    if( !m_connected ) return;
    stampAdmit();
    m_pin[0]->stampCurrent( m_volt*m_admit );
    m_pin[1]->stampCurrent(-m_volt*m_admit );
}

void Battery::setVoltage( double volt )
{
    if( volt < 1e-12 ) volt = 1e-12;
    m_volt = volt;
    m_changed = true;
}

void Battery::setResistance( double resist )
{
    if( resist < 1e-14 ) resist = 1e-14;
    m_admit = 1/resist;
    m_changed = true;
}

void Battery::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-7,-8,-7, 8 );
    p->drawLine(-2,-3,-2, 3 );
    p->drawLine( 3,-8, 3, 8 );
    p->drawLine( 8,-3, 8, 3 );

    Component::paintSelected( p );
}
