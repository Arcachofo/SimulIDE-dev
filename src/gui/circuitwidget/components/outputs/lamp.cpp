/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QtMath>

#include "lamp.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "pin.h"

#include "doubleprop.h"

Component* Lamp::construct( QObject* parent, QString type, QString id )
{ return new Lamp( parent, type, id ); }

LibraryItem* Lamp::libraryItem()
{
    return new LibraryItem(
        tr( "Incandescent lamp" ),
        "Other Outputs",
        "lamp.png",
        "Lamp",
        Lamp::construct);
}

Lamp::Lamp( QObject* parent, QString type, QString id )
    : Comp2Pin( parent, type, id )
    , eResistor( id )
{
    m_area = QRect(-10,-10, 20, 20 );

    m_maxCurrent = 0.2;
    setRes( 5 );

    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];

    Simulator::self()->addToUpdateList( this );
    Lamp::initialize();

    addPropGroup( { tr("Main"), {
new DoubProp<Lamp>( "Resistance", tr("Resistance"), "Ω", this, &Lamp::getRes, &Lamp::setResSafe ),
new DoubProp<Lamp>( "MaxCurrent", tr("Max Current"),"A", this, &Lamp::maxCurrent, &Lamp::setMaxCurrent ),
    },0 } );
}
Lamp::~Lamp(){}

void Lamp::initialize()
{
    m_crashed = false;
    m_warning = false;
    m_prevStep    = 0;
    m_avgCurrent  = 0.;
    m_intensity   = 25;
    m_brightness  = 0;
    m_totalCurrent = 0;
    m_lastPeriod   = 0;

    m_current = 0;
    m_lastCurrent = 0;
    update();
}

void Lamp::stamp()
{
    m_ePin[0]->changeCallBack( this );
    m_ePin[1]->changeCallBack( this );

    eResistor::stamp();
}

void Lamp::updateStep()
{
    uint32_t intensity = m_intensity;
    updateBright();

    if( overCurrent() > 1.5 )
    {
        m_warning = true;
        m_crashed = overCurrent() > 2;
        update();
    }else{
        if( m_warning ) update();
        m_warning = false;
        m_crashed = false;
    }
    if( intensity != m_intensity ) update();
}

void Lamp::voltChanged()
{
    m_current = qFabs( m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage() )*m_admit;
    updateVI();
}

void Lamp::updateVI()
{
    const uint64_t step = Simulator::self()->circTime();
    uint64_t period = (step-m_prevStep);
    m_prevStep = step;
    m_lastPeriod += period;

    if( m_lastCurrent > 0 ) m_totalCurrent += m_lastCurrent*period;
    m_lastCurrent = m_current;
}

void Lamp::updateBright()
{
    if( !Simulator::self()->isRunning() )
    {
        m_totalCurrent = 0;
        m_lastPeriod = 0;
        m_intensity = 25;
        return;
    }
    updateVI();

    uint64_t psPF = Simulator::self()->realPsPF();;

    if( m_lastPeriod > psPF/2 ) // Update 2 times per frame
    {
        m_avgCurrent = m_totalCurrent/m_lastPeriod;
        m_brightness = pow( m_avgCurrent/m_maxCurrent, 1.0/2.0 );

        m_totalCurrent  = 0;
        m_lastPeriod = 0;
        m_intensity  = uint32_t(m_brightness*255)+25;
}   }

void Lamp::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QColor color;

    if( m_warning/*m_current > m_maxCurrent*1.2*/ ) // Led overcurrent
    {
        p->setBrush( QColor( 255, 150, 0 ) );
        color = QColor( Qt::red );
    }
    if( m_crashed )  // Led extreme overcurrent
    {
        p->setBrush( Qt::white );
        color = QColor( Qt::white );
    }else{
        if( m_intensity > 25 )
        {
            m_intensity += 15;       // Set a Minimun Bright
            if( m_intensity > 255 ) m_intensity = 255;
            color = QColor( m_intensity, m_intensity, m_intensity*0.85 );
        }
        else color = QColor( 25, 25, 100 );
    }
    p->setBrush( color );
    p->drawEllipse(-8,-8, 16, 16 );

    p->drawLine(-5,-5, 5, 5 );
    p->drawLine(-5, 5, 5,-5 );
}
