/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "e-resistor.h"
#include "simulator.h"
#include "e-pin.h"

eResistor::eResistor( QString id )
         : eElement( id )
{
    m_admit  = 1.0/100.0;
    m_current = 0;
    m_ePin.resize(2);
}
eResistor::~eResistor(){}

void eResistor::stamp()
{
    m_ePin[0]->setEnodeComp( m_ePin[1]->getEnode() );
    m_ePin[1]->setEnodeComp( m_ePin[0]->getEnode() );
    stampAdmit();
}

void eResistor::stampAdmit()
{
    if( !m_ePin[0] ) return;
    if( !m_ePin[0]->isConnected() || !m_ePin[1]->isConnected() ) return;

    m_ePin[0]->stampAdmitance( m_admit );
    m_ePin[1]->stampAdmitance( m_admit );
}

void eResistor::setRes( double resist )
{
    if( resist < 1e-12 ) resist = 1e-12;
    setAdmit( 1/resist );
}

void eResistor::setAdmit( double admit )   // Admit can be 0
{
    m_admit = admit;
    stampAdmit();
}

void eResistor::setResSafe( double resist )
{
    Simulator::self()->pauseSim();
    eResistor::setRes( resist );
    Simulator::self()->resumeSim();
}

double eResistor::current()
{
    updateVI();
    return m_current;
}

void eResistor::updateVI()
{
    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage();
        m_current = volt*m_admit;
    }
    else m_current = 0;
}

