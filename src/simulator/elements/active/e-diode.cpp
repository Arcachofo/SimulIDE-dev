/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "e-diode.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

eDiode::eDiode( QString id )
      : eResistor( id )
{
    m_vt = 0.025865;
    m_vzCoef = 1/m_vt;

    m_resistor = new eResistor( m_elmId+"-resistor");
    m_midEnode = new eNode( m_elmId+"-mideNode");
}
eDiode::~eDiode()
{
    m_pinN->setEnode( NULL );
    m_pinR0->setEnode( NULL );
    delete m_resistor;
}

void eDiode::createSerRes() // --P[diode]N--(midEnode)--R0[resistor]R1--
{
    setNumEpins( 3 ); // This creates ePin1=pinN & ePin2=pinR0
    m_pinP  = m_ePin[0]; // m_ePin[0] created in upper class: Diode, Led, etc.
    m_pinN  = m_ePin[1];
    m_pinR0 = m_ePin[2];
//  m_pinR1 created and defined in upper class: Diode, Led, etc.

    m_resistor->setEpin( 0, m_pinR0 );
    m_resistor->setEpin( 1, m_pinR1 );

    m_pinN->setEnode( m_midEnode );
    m_pinR0->setEnode( m_midEnode );
}

void eDiode::initialize()
{
    m_converged = true;
    m_admit = m_bAdmit;
    m_voltPN = 0;
    m_current = 0;
}

void eDiode::stamp()
{
    eNode* node = m_pinP->getEnode();
    if( node ) node->addToNoLinList( this );

    node = m_pinN->getEnode();
    if( node ) node->addToNoLinList( this );

    eResistor::stamp();
}

void eDiode::voltChanged()
{
    double voltPN = m_pinP->getVolt() - m_pinN->getVolt();
    if( abs( voltPN - m_voltPN ) < .01 ) { m_step = 0; m_converged = true; return; } // Converged
    m_converged = false;
    Simulator::self()->notCorverged();

    m_step += .01;
    double gmin = m_bAdmit*exp( m_step );
    if( gmin > .1 ) gmin = .1;

    if( m_bkDown == 0 || voltPN >= 0 )  // Normal  Diode or Forward biased Zener
    {
        voltPN = limitStep( voltPN, m_vScale, m_vCriti );
        double eval = exp( voltPN*m_vdCoef );
        m_admit = m_satCur*m_vdCoef*eval + gmin;
        m_current = m_satCur*(eval-1);
    }
    else{                               // Reverse  biased Zener
        double volt = -voltPN-m_zOfset;
        voltPN = -( limitStep( volt, m_vt, m_vzCrit ) + m_zOfset );
        double eval = exp( voltPN*m_vdCoef );
        double expCoef = exp( volt*m_vzCoef );
        m_admit = m_satCur*( m_vdCoef*eval - m_vzCoef*m_vzCoef*expCoef ) + gmin;
        m_current = m_satCur*( eval - expCoef - 1 );
    }
    m_voltPN = voltPN;

    eResistor::stampAdmit();

    double stCurr = m_current - m_admit*voltPN;
    m_pinP->stampCurrent(-stCurr );
    m_pinN->stampCurrent( stCurr );
}

inline double eDiode::limitStep( double vnew, double scale, double vc )
{
    if( vnew > vc && abs( vnew - m_voltPN ) > scale*2 ) // check new voltage; has current changed by factor of e^2?
    {
        if( m_voltPN > 0 )
        {
            double arg = 1 + (vnew - m_voltPN)/scale;
            if( arg > 0 ) vnew = m_voltPN + scale*log( arg );
            else          vnew = vc;
        }else             vnew = scale*log( vnew/scale );
    } return vnew;
}

void eDiode::SetParameters( double sc, double ec, double bv, double sr )
{
    m_satCur = sc;
    m_emCoef = ec;
    m_bkDown = bv;
    setRes( sr );

    updateValues();
}

void eDiode::setThreshold( double fdDrop )
{
    m_satCur = 1/( exp( fdDrop*m_vdCoef) - 1 );
    updateValues();
}

void eDiode::setSatCur( double satCur )
{
    m_satCur = satCur;
    updateValues();
}

void eDiode::setEmCoef( double emCoef )
{
    m_emCoef = emCoef;
    updateValues();
}

void  eDiode::setBrkDownV( double bkDown )
{
    if( bkDown >= 0 ) m_bkDown = bkDown;
    else              m_bkDown = -bkDown;
    updateValues();
}

void eDiode::updateValues()
{
    m_vScale = m_emCoef*m_vt;
    m_vdCoef = 1/m_vScale;
    m_fdDrop = m_vScale*log( 1/m_satCur + 1 );
    m_vCriti = m_vScale*log( m_vScale/(sqrt(2)*m_satCur) );
    m_zOfset = m_bkDown - m_vt*log(-(1-0.005/m_satCur) );
    m_vzCrit = m_vt*log( m_vt/(sqrt(2)*m_satCur) );
    m_bAdmit = m_satCur*1e-2;
}

ePin* eDiode::getEpin( int num )
{
    if     ( num == 0 ) return m_ePin[0];
    else if( num == 1 ) return m_pinR1;
    return NULL;
}

void eDiode::setEpin( int num, ePin* pin )
{
    if     ( num == 0 ) m_ePin[0] = pin;
    else if( num == 1 ) m_pinR1 = pin;
}
