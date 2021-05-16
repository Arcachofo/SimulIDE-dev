/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "gate.h"
#include "connector.h"
#include "circuit.h"
#include "iopin.h"


Gate::Gate( QObject* parent, QString type, QString id, int inputs )
    : LogicComponent( parent, type, id )
    , eElement( id )
{
    setNumInps( inputs );                           // Create Input Pins
    m_outPin.resize( 1 );
    
    m_outPin[0] = new IoPin( 0, QPoint( 16, 0 ), m_id+"-out", 1, this, output );
    initPin( m_outPin[0] );
                          
    m_tristate = false;
    m_openCol  = false;
    m_rndPD = true; // Randomize Propagation Delay
}
Gate::~Gate(){}

QList<propGroup_t> Gate::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Inverted", tr("Inverted"),""} );
    mainGroup.propList.append( {"Open_Collector", tr("Open Drain"),""} );

    propGroup_t elecGroup { tr("Electric") };
    elecGroup.propList.append( {"Input_High_V", tr("Low to High Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Low_V", tr("High to Low Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Imped", tr("Input Impedance"),"Ω"} );
    elecGroup.propList.append( {"Out_High_V", tr("Output High Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Low_V", tr("Output Low Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Imped", tr("Output Impedance"),"Ω"} );

    propGroup_t edgeGroup { tr("Edges") };
    edgeGroup.propList.append( {"Tpd_ps", tr("Propagation Delay"),"ps"} );
    edgeGroup.propList.append( {"Tr_ps", tr("Rise Time"),"ps"} );
    edgeGroup.propList.append( {"Tf_ps", tr("Fall Time"),"ps"} );

    return {mainGroup, elecGroup, edgeGroup};
}

void Gate::stamp()
{
    LogicComponent::stamp( this );

    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_inPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_out = false;
}

void Gate::voltChanged()
{
    if( m_tristate ) LogicComponent::updateOutEnabled();

    int  inputs = 0;

    for( int i=0; i<m_numInputs; ++i )
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
    return (inputs==m_numInputs); // Default for: Buffer, Inverter, And, Nand
}

bool Gate::tristate() { return m_tristate; }

void Gate::setTristate( bool t ) { m_tristate = t; }

bool Gate::openCol() { return m_openCol; }

void Gate::setOpenCol( bool op )
{
    m_openCol = op;

    if( op ) m_outPin[0]->setPinMode( open_col );
    else     m_outPin[0]->setPinMode( output );
}

void Gate::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;

    for( int i=0; i<m_numInputs; i++ )
    {
        IoPin* pin = m_inPin[i];
        if( pin->connector() ) pin->connector()->remove();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        pin->reset();
        delete pin;
    }

    m_inPin.resize( inputs );

    for( int i=0; i<inputs; i++ )
    {
        m_inPin[i] = new IoPin( 180, QPoint(-8-8,-4*inputs+i*8+4 )
                               , m_id+"-in"+QString::number(i), i, this, input );
    }
    m_area = QRect( -20, -4*m_numInputs, 32, 4*2*m_numInputs );
    
    Circuit::self()->update();
}

void Gate::setInverted( bool inverted )
{
    LogicComponent::setInverted( inverted );
    Circuit::self()->update();
}

#include "moc_gate.cpp"
