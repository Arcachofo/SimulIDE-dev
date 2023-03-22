/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
// Based on Falstad Circuit Simulator BJT model: https://falstad.com

#include <QtMath>

#include "e-bjt.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

#define COLL m_ePin[0]
#define EMIT m_ePin[1]
#define BASE m_ePin[2]

eBJT::eBJT( QString id )
    : eElement( id )
    , m_BEjunction( id+"BEjunct")
    , m_BCjunction( id+"BCjunct")
{
    m_ePin.resize(3);

    m_PNP = false;

    m_gain = 100;
    m_rgain = .5;
    m_fgain = m_gain/(m_gain+1);

    m_vt = 0.025865;
    m_satCur = 1e-13;
    m_vCrit = m_vt*qLn( m_vt/(qSqrt(2)*m_satCur) );

    m_BEjunction.setNumEpins( 2 );
    m_BE = m_BEjunction.getEpin( 0 );
    m_EB = m_BEjunction.getEpin( 1 );

    m_BCjunction.setNumEpins( 2 );
    m_BC = m_BCjunction.getEpin( 0 );
    m_CB = m_BCjunction.getEpin( 1 );

}
eBJT::~eBJT(){}

void eBJT::stamp()
{
    m_step = 0;
    m_voltBE = 0;
    m_voltBC = 0;
    m_baseCurr = 0;

    m_EC = EMIT;
    m_CE = COLL;

    eNode* collNod = COLL->getEnode();// Collector
    eNode* emitNod = EMIT->getEnode();// Emitter
    eNode* baseNod = BASE->getEnode();// Base

    if( collNod ) collNod->addToNoLinList( this );
    if( emitNod ) emitNod->addToNoLinList( this );
    if( baseNod ) baseNod->addToNoLinList( this );

    m_BE->setEnode( baseNod );
    m_BE->setEnodeComp( emitNod );
    m_EB->setEnode( emitNod );
    m_EB->setEnodeComp( baseNod );

    m_BC->setEnode( baseNod );
    m_BC->setEnodeComp( collNod );
    m_CB->setEnode( collNod );
    m_CB->setEnodeComp( baseNod );

    m_CE->setEnodeComp( emitNod );
    m_EC->setEnodeComp( collNod );

    BASE->createCurrent();
    COLL->createCurrent();
    EMIT->createCurrent();
}

void eBJT::voltChanged()
{
    double pnp = m_PNP ? -1 : 1;
    double voltC = COLL->getVoltage();
    double voltE = EMIT->getVoltage();
    double voltB = BASE->getVoltage();
    double voltBC = voltB-voltC;
    double voltBE = voltB-voltE;

    if( m_changed ) m_changed = false;
    else if( qFabs(voltBC-m_voltBC) < .01
          && qFabs(voltBE-m_voltBE) < .01 )
        { m_step = 0; return; }
    Simulator::self()->notCorverged();

    m_step += .1;
    double gmin = m_satCur*1e-2*qExp( m_step );
    if( gmin > .1 ) gmin = .1;

    voltBC = pnp*limitStep( pnp*voltBC, pnp*m_voltBC );
    m_voltBC = voltBC;
    voltBE = pnp*limitStep( pnp*voltBE, pnp*m_voltBE );
    m_voltBE = voltBE;

    double pcoef = pnp/m_vt;
    double expBC = qExp( voltBC*pcoef );
    double expBE = qExp( voltBE*pcoef );

    double ie = pnp*m_satCur*(-(expBE-1)/m_fgain + (expBC-1) );
    double ic = pnp*m_satCur*( (expBE-1) - (expBC-1)/m_rgain );
    m_baseCurr = -(ie+ic);

    double Gee = -m_satCur/m_vt*expBE/m_fgain;
    double Gcc = -m_satCur/m_vt*expBC/m_rgain;
    double Gce = -Gee*m_fgain;
    double Gec = -Gcc*m_rgain;

    Gcc -= gmin;
    Gee -= gmin;

    // Admitance Matrix OK
    m_BC->stampAdmitance(-Gec-Gcc );
    m_CB->stampAdmitance(-Gce-Gcc );
    m_BE->stampAdmitance(-Gee-Gce );
    m_EB->stampAdmitance(-Gee-Gec );
    m_CE->stampAdmitance( Gce );
    m_EC->stampAdmitance( Gec );

    BASE->stampCurrent(-m_baseCurr - (Gec+Gcc)*voltBC - (Gee+Gce)*voltBE );
    COLL->stampCurrent(-ic + Gce*voltBE + Gcc*voltBC );
    EMIT->stampCurrent(-ie + Gee*voltBE + Gec*voltBC );
}

double eBJT::limitStep( double vnew, double vold )
{
    if( vnew > m_vCrit && qFabs(vnew-vold) > (2*m_vt) ){
        if( vold > 0 ){
            double arg = 1+(vnew-vold)/m_vt;
            if( arg > 0 )  vnew = vold + m_vt*qLn( arg );
            else           vnew = m_vCrit;
        }
        else vnew = m_vt*qLn( vnew/m_vt );
    }
    return vnew;
}

void eBJT::setGain( double gain )
{
    m_gain = gain;
    m_fgain = m_gain/(m_gain+1);
    m_changed = true;
}

void eBJT::setThreshold( double vCrit )
{
    m_vCrit = vCrit;
    m_satCur = m_vt/(qExp( vCrit/m_vt )*qSqrt(2));
    m_changed = true;
}
