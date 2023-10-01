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

    m_Vd = 0;
    m_Vs = 0;
    m_Vg = 0;

    m_step = 0;
    m_gateV = 0;

    m_kRDSon = m_RDSon*(10-m_threshold);
    m_Gth    = m_threshold-m_threshold/4;

    if( (m_ePin[0]->isConnected())
      &&(m_ePin[1]->isConnected()) )
    {
        eResistor::stamp();

        m_ePin[0]->createCurrent();
        m_ePin[1]->createCurrent();

        m_ePin[0]->getEnode()->addToNoLinList(this);
        m_ePin[1]->getEnode()->addToNoLinList(this);
    }
    if( m_ePin[2]->isConnected() ) m_ePin[2]->getEnode()->addToNoLinList(this);
}

void eMosfet::voltChanged()
{
    double _Vd = m_ePin[0]->getVoltage();
    double _Vs = m_ePin[1]->getVoltage();
    double _Vg = m_ePin[2]->isConnected() ? m_ePin[2]->getVoltage() : _Vs;
    double Vd = _Vd;
    double Vs = _Vs;
    double Vg = _Vg;

    if( converged( m_Vd, Vd )
     && converged( m_Vs, Vs )
     && converged( m_Vg, Vg) ) { m_step = 0; return; } // Converged
    m_step += 0.0001;
    Simulator::self()->notCorverged();

    if( (Vd-m_Vd) > 0.5 ) Vd = m_Vd + 0.5;
    if( (m_Vd-Vd) > 0.5 ) Vd = m_Vd - 0.5;
    if( (Vs-m_Vs) > 0.5 ) Vs = m_Vs + 0.5;
    if( (m_Vs-Vs) > 0.5 ) Vs = m_Vs - 0.5;

    m_Vd = _Vd;
    m_Vs = _Vs;
    m_Vg = _Vg;

    double Vgs = Vg-Vs;
    double Vds = Vd-Vs;

    if( m_Pchannel ){ Vgs = -Vgs; Vds = -Vds; }
    if( m_depletion ) Vgs = -Vgs;
    
    double gateV = Vgs - m_Gth;
    
    if( gateV < 0 ) gateV = 0;

    double admit = m_depletion ? 1/m_RDSon : cero_doub;
    double maxCurrDS = Vds*m_admit;
    double current  = 0;
    
    if( gateV > 0 )
    {
        admit = m_depletion ? cero_doub : 1/m_RDSon;
        double satK = 1+Vds/100;
        if( Vds > gateV ) Vds =gateV;

        double DScurrent = (gateV*Vds-qPow(Vds,2)/2)*satK/m_kRDSon;
        if( DScurrent > maxCurrDS ) DScurrent = maxCurrDS;
        current = m_depletion ? DScurrent : (maxCurrDS-DScurrent);
    }
    if( m_Pchannel ) current = -current;
    
    if( admit != m_admit ){
        eResistor::setAdmit( admit );
        eResistor::stamp();
    }
    m_gateV = gateV;

    m_ePin[0]->stampCurrent( current );
    m_ePin[1]->stampCurrent(-current );
}

bool eMosfet::converged( double pre, double post )
{
    double delta = qFabs( post-pre );

    if( m_RDSon < 1 ) delta *= 100;

    if( delta < 0.01 + m_step
     || delta < qFabs( post )/1000 ) return true;

    return false;
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

