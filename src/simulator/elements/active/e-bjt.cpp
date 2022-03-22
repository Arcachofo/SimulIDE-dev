/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
// Based on Falstad Circuit Simulator Diode model: https://falstad.com

#include <math.h>

#include "e-bjt.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

#define COLL m_ePin[0]
#define EMIT m_ePin[1]
#define BASE m_ePin[2]

eBJT::eBJT( QString id )
    : eElement( id )
{
    m_ePin.resize(3);

    m_gain = 100;
    m_rgain = .5;
    m_fgain = m_gain/(m_gain+1);
    m_PNP = false;

    m_vt = 0.025865;
    m_satCur = 1e-13;
    m_vCrit = m_vt*log( m_vt/(sqrt(2)*m_satCur) );

    m_BCjunction = new eElement( id+"BCjunct");
    m_BCjunction->setNumEpins( 2 );
    m_BC = m_BCjunction->getEpin( 0 );
    m_CB = m_BCjunction->getEpin( 1 );

    m_BEjunction = new eElement( id+"BEjunct");
    m_BEjunction->setNumEpins( 2 );
    m_BE = m_BEjunction->getEpin( 0 );
    m_EB = m_BEjunction->getEpin( 1 );
}
eBJT::~eBJT()
{
    delete m_BEjunction;
    delete m_BCjunction;
}

void eBJT::initialize()
{
    //m_accuracy = Simulator::self()->NLaccuracy()/100;
    m_step = 0;
    m_voltBE = 0;
    m_voltBC = 0;
    m_baseCurr = 0;

    m_EC = EMIT;
    m_CE = COLL;

    m_BE->setEnode( NULL );
    m_BE->setEnodeComp( NULL );
    m_EB->setEnode( NULL );
    m_EB->setEnodeComp( NULL );

    m_BC->setEnode( NULL );
    m_BC->setEnodeComp( NULL );
    m_CB->setEnode( NULL );
    m_CB->setEnodeComp( NULL );
}

void eBJT::stamp()
{
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

    //double r = ((double)(std::rand() %5))*1e-2; // Random start diference
    m_BE->stampAdmitance( cero_doub );
    m_EB->stampAdmitance( cero_doub );
    m_BC->stampAdmitance( cero_doub );
    m_CB->stampAdmitance( cero_doub );
    m_CE->stampAdmitance( cero_doub );
    m_EC->stampAdmitance( cero_doub );
}

void eBJT::voltChanged()
{
    double pnp = m_PNP ? -1 : 1;
    double voltC = COLL->getVolt();
    double voltE = EMIT->getVolt();
    double voltB = BASE->getVolt();
    double voltBC = voltB-voltC;
    double voltBE = voltB-voltE;

    if( (fabs(voltBC-m_voltBC) < .01) && (fabs(voltBE-m_voltBE) < .01) )
        { m_step = 0; return; }
    Simulator::self()->notCorverged();

    m_step += .1;
    double gmin = m_satCur*1e-2*exp( m_step );
    if( gmin > .1 ) gmin = .1;

    voltBC = pnp*limitStep( pnp*voltBC, pnp*m_voltBC );
    m_voltBC = voltBC;
    voltBE = pnp*limitStep( pnp*voltBE, pnp*m_voltBE );
    m_voltBE = voltBE;

    double pcoef = pnp/m_vt;
    double expBC = exp( voltBC*pcoef );
    double expBE = exp( voltBE*pcoef );

    double ie = pnp*m_satCur*(-(expBE-1)/m_fgain + (expBC-1) );
    double ic = pnp*m_satCur*( (expBE-1) - (expBC-1)/m_rgain );
    m_baseCurr = -(ie+ic);

    double Gee = -m_satCur/m_vt*expBE/m_fgain-gmin;
    double Gcc = -m_satCur/m_vt*expBC/m_rgain-gmin;
    double Gce = -Gee*m_fgain;
    double Gec = -Gcc*m_rgain;

    m_BC->stampAdmitance(-Gec-Gcc );
    m_CB->stampAdmitance(-Gce-Gcc  );
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
    if( vnew > m_vCrit && fabs(vnew-vold) > (2*m_vt) ){
        if( vold > 0 ){
            double arg = 1+(vnew-vold)/m_vt;
            if( arg > 0 )  vnew = vold + m_vt*log( arg );
            else           vnew = m_vCrit;
        }
        else vnew = m_vt *log( vnew/m_vt );
    }
    return vnew;
}

void eBJT::setGain( double gain )
{
    m_gain = gain;
    m_fgain = m_gain/(m_gain+1);
}

void eBJT::setThreshold( double vCrit )
{
    m_vCrit = vCrit;
    m_satCur = m_vt/(exp( vCrit/m_vt )*sqrt(2));
}
