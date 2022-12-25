/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "simulator.h"
#include "e-coil.h"
#include "e-node.h"
#include "e-pin.h"

#include <math.h>

eCoil::eCoil( int i, int s, int t, double h, double r, QString id )
     : eResistor( id )
     , m_resistor0( id+"resistor0")
     , m_resistor1( id+"resistor1")
{
    index = i;
    sign  = s;
    size = t;
    relation = r;
    inductance = h*r*r; // H

    setNumEpins( 2 );
    m_resistor0.setNumEpins( 2 );
    m_resistor1.setNumEpins( 2 );

    m_resistor0.setAdmit( 1000 );
    m_resistor1.setAdmit( 1000 );
}
eCoil::~eCoil(){}

void eCoil::initialize()
{
    m_enode0 = new eNode( m_elmId+"enode0");
    m_enode1 = new eNode( m_elmId+"enode1");
}

void eCoil::stamp(){}
void eCoil::stampCoil()
{
    m_ePin[0]->setEnode( m_enode0 );
    m_resistor0.getEpin(1)->setEnode( m_enode0 );

    m_ePin[1]->setEnode( m_enode1 );
    m_resistor1.getEpin(1)->setEnode( m_enode1 );

    m_ePin[0]->createCurrent();
    m_ePin[1]->createCurrent();

    m_currSource = 0;
    eResistor::stamp();
}

void eCoil::setEnode( int n, eNode* e )
{
    if( n == 0 ) m_resistor0.getEpin(0)->setEnode( e );
    else         m_resistor1.getEpin(0)->setEnode( e );
}

double eCoil::getVolt()
{
    double volt = m_ePin[0]->getVoltage() - m_ePin[1]->getVoltage();
    return volt;
}

void eCoil::stampCurrent( double current )
{
    m_currSource += current;

    m_ePin[0]->stampCurrent(-m_currSource );
    m_ePin[1]->stampCurrent( m_currSource );
}

void eCoil::addIductor( eCoil* coil, double g )
{
    int n0 = coil->getEpin( 0 )->getEnode()->getNodeNumber();
    int n1 = coil->getEpin( 1 )->getEnode()->getNodeNumber();

    m_ePin[0]->addSingAdm( n0,-g );
    m_ePin[0]->addSingAdm( n1, g );
    m_ePin[1]->addSingAdm( n0, g );
    m_ePin[1]->addSingAdm( n1,-g );
}
// Coil structure: (VVVV = resistor)
//
// ┏━-ePin0-eNode0-VVVV-━━⚫ Pin0 -- eNode
// ┃
// ┗━-ePin1-eNode1-VVVV-━━⚫ Pin1 -- eNode
