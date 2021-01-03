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

#include "e-bjt.h"
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

    m_vt = .025;
    m_rsCurr = 1e-13;
    m_thr = m_vt*log( m_vt/(sqrt(2)*m_rsCurr) );

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
    m_accuracy = Simulator::self()->NLaccuracy()/100;

    m_steps = 0;
    m_voltBE = 0;
    m_voltBC = 0;

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

    double r = ((double)(std::rand() %5))*1e-2;
    m_BE->stampAdmitance( cero_doub+r );
    m_EB->stampAdmitance( cero_doub+r );
    m_BC->stampAdmitance( cero_doub );
    m_CB->stampAdmitance( cero_doub );
    m_CE->stampAdmitance( cero_doub );
    m_EC->stampAdmitance( cero_doub );
}

void eBJT::voltChanged()
{
    double voltC = COLL->getVolt();
    double voltE = EMIT->getVolt();
    double voltB = BASE->getVolt();
    double voltBC = voltB-voltC;
    double voltBE = voltB-voltE;
    if( m_PNP ) { voltBC = -voltBC; voltBE = -voltBE; }

    else if( (fabs(voltBC-m_voltBC) < .01) && (fabs(voltBE-m_voltBE) < .01) )
    { m_steps = 0; return; }

    double gmin = 0;//1e-15;
    if( ++m_steps > 100 )
    {
        gmin = exp(-9*log(10)*(1-m_steps/300.));
        if (gmin > .1) gmin = .1;
    }
    voltBC = limitStep( voltBC, m_voltBC );
    m_voltBC = voltBC;
    voltBE = limitStep( voltBE, m_voltBE );
    m_voltBE = voltBE;

    double expBC = exp( voltBC/m_vt );
    double expBE = exp( voltBE/m_vt );

    double ie = m_rsCurr*(-(expBE-1)+m_rgain*(expBC-1));
    double ic = m_rsCurr*(m_fgain*(expBE-1)-(expBC-1));
    if( m_PNP ) { ie = -ie; ic = -ic; }
    m_baseCurr = -(ie+ic);

    double Gee = -m_rsCurr/m_vt*expBE;
    double Gcc = -m_rsCurr/m_vt*expBC;
    double Gce = -Gee*m_fgain;
    double Gec = -Gcc*m_rgain;

    m_BC->stampAdmitance( -Gec-Gcc + gmin );
    m_CB->stampAdmitance( -Gec-Gce + gmin );

    m_BE->stampAdmitance( -Gee-Gce + gmin );
    m_EB->stampAdmitance( -Gee-Gcc + gmin );

    m_CE->stampAdmitance( Gce );
    m_EC->stampAdmitance( Gcc );

    double GcevBE = Gce*voltBE;
    double GcevBC = Gec*voltBC;
    double GeevBE = Gee*voltBE;
    double GccvBC = Gcc*voltBC;

    BASE->stampCurrent( ie + ic - GcevBE-GcevBC - GeevBE-GccvBC );
    COLL->stampCurrent( -ic + GcevBE + GcevBC );
    EMIT->stampCurrent( -ie + GeevBE + GccvBC );
}

double eBJT::limitStep( double vnew, double vold )
{
    if( vnew > m_thr && fabs(vnew-vold) > (2*m_vt) )
    {
        if( vold > 0 )
        {
            double arg = 1+(vnew-vold)/m_vt;
            if( arg > 0 )  vnew = vold + m_vt*log( arg );
            else           vnew = m_thr;
        }
        else vnew = m_vt *log( vnew/m_vt );
    }
    return vnew;
}

