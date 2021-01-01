/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "e-op_amp.h"
#include "e-source.h"
#include "simulator.h"

eOpAmp::eOpAmp( QString id )
      : eElement( id )
{
    m_ePin.resize(5);

    m_gain = 1000;
    m_voltPosDef = 5;
    m_voltNegDef = 0;
    
    initialize();
}
eOpAmp::~eOpAmp()
{
    delete m_output;
}

void eOpAmp::initialize()
{
    m_accuracy = Simulator::self()->NLaccuracy();

    m_lastOut = 0;
    m_lastIn  = 0;
    m_k = 1e-6/m_gain;
    m_firstStep = true;
}

void eOpAmp::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToNoLinList(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToNoLinList(this);
    if( m_ePin[2]->isConnected() ) m_ePin[2]->getEnode()->addToNoLinList(this);
}

void eOpAmp::voltChanged() // Called when input pins nodes change volt
{
    if( m_powerPins )
    {
        m_voltPos = m_ePin[3]->getVolt();
        m_voltNeg = m_ePin[4]->getVolt();
    }
    else
    {
        m_voltPos = m_voltPosDef;
        m_voltNeg = m_voltNegDef;
    }
    double vd = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    //qDebug() << "lastIn " << m_lastIn << "vd " << vd ;
    
    double out = vd * m_gain;
    if     ( out > m_voltPos ) out = m_voltPos;
    else if( out < m_voltNeg ) out = m_voltNeg;
    
    //qDebug() << "lastOut " << m_lastOut << "out " << out << abs(out-m_lastOut)<< "<1e-5 ??";

    if( fabs(out-m_lastOut) < m_accuracy )
    {
        m_firstStep = true;
        return;
    }

    if( m_firstStep )                  // First step after a convergence
    {
        double dOut = -1e-6;           // Do a tiny step to se what happens
        if( vd>0 ) dOut = 1e-6;
        
        out = m_lastOut + dOut;
        m_firstStep = false;
    }
    else
    {
        if( m_lastIn != vd ) // We problably are in a close loop configuration
        {
            double dIn  = fabs(m_lastIn-vd); // Input diff with last step
            
            // Guess next converging output:
            out = (m_lastOut*dIn + vd*1e-6)/(dIn + m_k);
        }
        m_firstStep = true;
    }
    if     ( out >= m_voltPos ) out = m_voltPos;
    else if( out <= m_voltNeg ) out = m_voltNeg;
    
    //qDebug()<< "lastOut " << m_lastOut << "out " << out << "dOut" << dOut  << "converged" << m_firstStep;
    m_lastIn  = vd;
    m_lastOut = out;
    
    m_ePin[2]->stampCurrent( out/cero_doub );
}

double eOpAmp::gain()                {return m_gain;}
void   eOpAmp::setGain( double gain ){m_gain = gain;}

bool eOpAmp::hasPowerPins()          {return m_powerPins;}
void eOpAmp::setPowerPins( bool set ){m_powerPins = set;}

