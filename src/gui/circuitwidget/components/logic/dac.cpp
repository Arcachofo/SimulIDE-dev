/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include "dac.h"
#include "itemlibrary.h"
#include "simulator.h"

Component* DAC::construct( QObject* parent, QString type, QString id )
{
    return new DAC( parent, type, id );
}

LibraryItem* DAC::libraryItem()
{
    return new LibraryItem(
        tr( "DAC" ),
        tr( "Logic/Other Logic" ),
        "3to1.png",
        "DAC",
        DAC::construct );
}

DAC::DAC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
   , eElement( id )
{    
    m_width  = 4;
    m_height = 9;

    setNumInps( 8 );       // Create Input Pins
    setMaxVolt( 5 );

    LogicComponent::setNumOuts( 1 );
    
    m_outPin[0] = new IoPin( 0, QPoint( 24, -8 ), m_id+"-out", 1, this, output );
    m_outPin[0] ->setLabelText( "Out " );
    m_outPin[0] ->setLabelColor( QColor( 0, 0, 0 ) );
}
DAC::~DAC(){}

QList<propGroup_t> DAC::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Num_Bits", tr("Size"),"Bits"} );
    mainGroup.propList.append( {"Vref", tr("Reference Voltage"),"V"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    for( int i=0;i<4; ++i ) pg.first().propList.removeLast(); //remove Outputs.
    pg.prepend( mainGroup );
    return pg;
}

void DAC::stamp()
{
    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_inPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_outPin[0]->setOutState( true );
    m_value = -1;
}

void DAC::voltChanged()
{
    m_value = 0;

    for( int i=0; i<m_numOutputs; ++i )
        if( m_inPin[i]->getInpState() ) m_value += pow( 2, m_numInputs-1-i );

    Simulator::self()->addEvent( m_propDelay, this );
}

void DAC::runEvent()
{
    double v = m_maxVolt*m_value/m_maxValue;

    m_outPin[0]->setOutHighV( v );
    m_outPin[0]->stampOutput();
}

void DAC::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;

    LogicComponent::setNumInps( inputs );

    for( int i=0; i<inputs; i++ )
    {
        QString num = QString::number( inputs-i-1 );
        m_inPin[i] = new IoPin( 180, QPoint(-24,-8*inputs+i*8+8 ), m_id+"-in"+num, i, this, input );

        m_inPin[i]->setLabelText( "D"+num+" " );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
    }
    m_maxValue = pow( 2, m_numInputs )-1;

    m_height = inputs+1;
    m_area = QRect( -(m_width/2)*8, -m_height*8+8, m_width*8, m_height*8 );
}

#include "moc_dac.cpp"
