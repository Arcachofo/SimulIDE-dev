/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "gate.h"
#include "connector.h"
#include "circuit.h"
#include "iopin.h"

#include "boolprop.h"

Gate::Gate( QObject* parent, QString type, QString id, int inputs )
    : LogicComponent( parent, type, id )
{
    m_width = 2;
    m_initState = false;
    m_minInputs = inputs;

    setNumOuts( 1, "", 0, false );
    setNumInputs( inputs );  // Create Input Pins

    /// m_rndPD = true; // Randomize Propagation Delay:
}
Gate::~Gate(){}

QList<ComProperty*> Gate::edgeProps()
{
    QList<ComProperty*> edge = IoComponent::edgeProps();
    edge.prepend(
new BoolProp<Gate>( "initHigh", tr("Initial High State"),"", this, &Gate::initState, &Gate::setInitState ) );

    return edge;
}

void Gate::stamp()
{
    LogicComponent::stamp();
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );

    m_outPin[0]->setOutState( m_initState );
    m_out = m_initState;
    m_nextOutVal = m_outValue = m_initState;
}

void Gate::voltChanged()
{
    if( m_tristate ) LogicComponent::updateOutEnabled();

    int inputs = 0;

    for( uint i=0; i<m_inPin.size(); ++i )
    {
        bool state = m_inPin[i]->getInpState();
        if( state ) inputs++;
    }
    bool out = calcOutput( inputs ); // In each gate type

    m_nextOutVal = out? 1:0;
    if( m_out == out && !m_tristate ) return;
    m_out = out;

    sheduleOutPuts( this );
}

bool Gate::calcOutput( int inputs )
{
    return ((uint)inputs == m_inPin.size()); // Default for: Buffer, Inverter, And, Nand
}

void Gate::setNumInputs( int inputs )
{
    if( inputs < m_minInputs ) return;
    IoComponent::setNumInps( inputs, "" );
    m_outPin[0]->setY( 0 );
    m_area = QRect( -11, -4*m_inPin.size(), 19, 4*2*m_inPin.size() );
}
