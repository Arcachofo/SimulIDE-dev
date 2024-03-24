/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "reactive.h"
#include "simulator.h"
#include "propdialog.h"
#include "e-node.h"
#include "pin.h"

#define tr(str) simulideTr("Reactive",str)

Reactive::Reactive( QString type, QString id )
        : Comp2Pin( type, id )
        , eReactive( id )
{
    // Pin0--eReactive--ePin1--midEnode--ePin2--eResistor--Pin1
    m_ePin[0] = m_pin[0];
    setNumEpins( 3 );

    m_resistor = new eResistor( m_elmId+"-resistor");
    m_resistor->setRes( 1e-6 );
    m_resistor->setEpin( 0, m_ePin[2] );
    m_resistor->setEpin( 1, m_pin[1] );

    Simulator::self()->addToUpdateList( this );
}
Reactive::~Reactive()
{
    delete m_resistor;
}

void Reactive::initialize()
{
    m_crashed = false;
    m_warning = false;

    m_midEnode = new eNode( m_elmId+"-mideNode");
}

void Reactive::stamp()
{
    m_ePin[1]->setEnode( m_midEnode );
    m_ePin[2]->setEnode( m_midEnode );

    if( m_pin[0]->isConnected() && m_pin[1]->isConnected() )
        eReactive::stamp();
}

void Reactive::updateStep()
{
    if( m_changed )
    {
        m_changed = false;
        updtReactStep();
        if( m_propDialog ) m_propDialog->updtValues();
    }
    update();
}

void Reactive::setValue( double c )
{
    m_value = c;
    setCurrentValue( c );
}

void Reactive::setResist( double resist )
{
    m_resistor->setResSafe( resist );
}

void Reactive::setReaStep( double r )
{
    m_reacStep = r*1e12;
    m_changed = true;
}

void Reactive::setLinkedValue( double v, int i )
{
    setCurrentValue( m_value*v/1000 );
}
