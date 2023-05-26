/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "reactive.h"
#include "simulator.h"
#include "e-node.h"
#include "pin.h"

Reactive::Reactive( QObject* parent, QString type, QString id )
        : Comp2Pin( parent, type, id )
        , eReactive( id )
{
    m_area = QRectF( -10, -10, 20, 20 );

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
    if( m_warning != m_stepError )
    {
        m_warning = m_stepError;
        if( m_stepError ){
            setToolTip(tr("\n  Warnig: Capacitor can't update fast enough  \n\n"
                          "  Set auto step >= 1 for this capacitor  \n") );
                          //"  Or set Reactive Step <= ")+QString::number( m_deltaTime )+" ps globally  \n");
        }
        else setToolTip("");
    }
    if( m_warning ) update();

    uint64_t reactStep = Simulator::self()->reactStep();
    if( m_reacStep != reactStep )
    {
        m_reacStep = reactStep; // Time in ps
        m_changed = true;
    }
    if( m_changed )
    {
        m_changed = false;
        m_warning = false;
        m_stepError = false;
        updtReactStep();
    }
}

void Reactive::setValue( double c )
{
    m_value = c;
    m_changed = true;
}

void Reactive::setResist( double resist )
{
    m_resistor->setResSafe( resist );
}

void Reactive::setAutoStep( int a )
{
    if( a > 2 ) a = 2;
    m_autoStep = a;
    m_changed = true;
}
