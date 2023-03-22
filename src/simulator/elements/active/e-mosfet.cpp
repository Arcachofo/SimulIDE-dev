/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>   // qFabs(x,y)
//#include <QDebug>

#include "e-mosfet.h"
#include "simulator.h"
#include "e-pin.h"
#include "e-node.h"

eMosfet::eMosfet( QString id )
       : eResistor( id )
{
    m_Pchannel  = false;
    m_depletion = false;
    
    m_gateV     = 0;
    m_RDSon     = 1;
    m_threshold = 3;

    m_ePin.resize(3);
}
eMosfet::~eMosfet(){}

void eMosfet::stamp()
{
    m_admit = 1/m_RDSon;

    m_step = 0;
    m_gateV = 0;
    m_lastCurrent = 0;

    m_kRDSon = m_RDSon*(10-m_threshold);
    m_Gth    = m_threshold-m_threshold/4;
    m_accuracy = 5e-6;

    if( (m_ePin[0]->isConnected())
      &&(m_ePin[1]->isConnected())
      &&(m_ePin[2]->isConnected()) )
    {
        eResistor::stamp();

        m_ePin[0]->createCurrent();
        m_ePin[1]->createCurrent();

        m_ePin[0]->getEnode()->addToNoLinList(this);
        m_ePin[1]->getEnode()->addToNoLinList(this);
        m_ePin[2]->getEnode()->addToNoLinList(this);
    }
}

void eMosfet::voltChanged()
{
    double Vd = m_ePin[0]->getVoltage();
    double Vs = m_ePin[1]->getVoltage();
    double Vg = m_ePin[2]->getVoltage();
    double Vgs = Vg-Vs;
    double Vds = Vd-Vs;

    if( m_Pchannel ){ Vgs = -Vgs; Vds = -Vds; }
    if( m_depletion ) Vgs = -Vgs;
    
    double gateV = Vgs - m_Gth;
    
    if( gateV < 0 ) gateV = 0;

    double admit = cero_doub;
    double current = 0;
    double maxCurrDS = Vds*m_admit;
    double DScurrent = m_accuracy*2;
    
    if( gateV > 0 )
    {
        admit = 1/m_RDSon;
        double satK = 1+Vds/100;
        if( Vds > gateV ) Vds =gateV;

        DScurrent = (gateV*Vds-qPow(Vds,2)/2)*satK/m_kRDSon;
        if( DScurrent > maxCurrDS ) DScurrent = maxCurrDS;
        current = maxCurrDS-DScurrent;
    }
    if( m_Pchannel ) current = -current;
    
    if( admit != m_admit ){
        eResistor::setAdmit( admit );
        eResistor::stamp();
    }
    m_gateV = gateV;

    if( qFabs(current-m_lastCurrent)<m_accuracy ) return; // Converged
    Simulator::self()->notCorverged();

    m_lastCurrent = current;
    m_ePin[0]->stampCurrent( current );
    m_ePin[1]->stampCurrent(-current );
}

void eMosfet::setRDSon( double rdson )
{
    if( rdson < cero_doub ) rdson = cero_doub;
    if( rdson > 1000 ) rdson = 1000;
    m_RDSon = rdson;
    m_changed = true;
}

void eMosfet::setThreshold( double th )
{
    if( th < 0.01 ) return;
    m_threshold = th; 
    m_kRDSon = m_RDSon*(10-m_threshold);
    m_Gth = m_threshold-m_threshold/4;
    m_changed = true;
}

