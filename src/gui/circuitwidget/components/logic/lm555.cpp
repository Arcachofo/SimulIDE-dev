/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <math.h>

#include "lm555.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "iopin.h"
#include "e-node.h"

Component* Lm555::construct( QObject* parent, QString type, QString id )
{ return new Lm555( parent, type, id ); }

LibraryItem* Lm555::libraryItem()
{
    return new LibraryItem(
        tr( "lm555" ),
        tr( "Logic/Other Logic" ),
        "ic2.png",
        "Lm555",
        Lm555::construct );
}

Lm555::Lm555( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
     , m_ePinA( (id+"-ePinA"), 0 )
     , m_ePinB( (id+"-ePinB"), 0 )
     , m_ePinC( (id+"-ePinC"), 0 )
     , m_ePinD( (id+"-ePinD"), 0 )
     , m_resA(  (id+"-resA") )
     , m_resB(  (id+"-resB") )
     , m_resD(  (id+"-resD") )
{
    m_area = QRect( 0, 0, 8*4, 8*5 );
    m_color = QColor( 50, 50, 70 );
    
    m_pin.resize( 8 );
    
    m_Gnd = new Pin( 180, QPoint(-8, 8*1), id+"-ePin0", 0, this );
    m_Gnd->setLabelText( "Gnd" );
    m_pin[0] = m_Gnd;

    m_trigger = new Pin( 180, QPoint(-8, 8*2), id+"-ePin1", 1, this );
    m_trigger->setLabelText( "Trg" );
    m_pin[1] = m_trigger;

    m_output = new IoPin( 180, QPoint(-8, 8*3), id+"-ePin2", 2, this, output );
    m_output->setLabelText( "Out" );
    m_output->setOutputImp( 10 );
    m_output->setOutLowV( 0 );
    m_output->setOutHighV( 5-1.7 );
    m_output->setOutState( true );
    m_pin[2] = m_output;

    m_Reset = new Pin( 180, QPoint(-8, 8*4), id+"-ePin3", 3, this );
    m_Reset->setLabelText( "Rst" );
    m_pin[3] = m_Reset;

    m_cv = new Pin( 0, QPoint(4*8+8, 8*4), id+"-ePin4", 4, this );
    m_cv->setLabelText( "CV" );
    m_pin[4] = m_cv;

    m_threshold = new Pin( 0, QPoint(4*8+8, 8*3), id+"-ePin5", 5, this );
    m_threshold->setLabelText( "Thr" );
    m_pin[5] = m_threshold;

    m_discharge = new Pin( 0, QPoint(4*8+8, 8*2), id+"-ePin6", 6, this );
    m_discharge->setLabelText( "Dis" );
    m_pin[6] = m_discharge;

    m_Vcc = new Pin( 0, QPoint(4*8+8, 8*1),id+"-ePin7", 7, this );
    m_Vcc->setLabelText( "Vcc" );
    m_pin[7] = m_Vcc;

    m_thrEnode = NULL;
    m_resA.setEpin( 0, m_Vcc );
    m_resA.setEpin( 1, &m_ePinA );

    m_resB.setEpin( 0, &m_ePinB );
    m_resB.setEpin( 1, &m_ePinC );

    m_resD.setEpin( 0, &m_ePinD );
    m_resD.setEpin( 1, m_discharge );

    m_propDelay = 10*1000; // 10 ns
}
Lm555::~Lm555(){}

void Lm555::initialize()
{
    m_outState = false;
    m_voltNeg = 0;
    m_voltPos = 0;
}

void Lm555::attach()
{
    eNode* enod = m_cv->getEnode();   // Control Voltage Pin
    if( !enod )                       // Not connected: Create threshold eNode
    {
        m_thrEnode = new eNode( m_id+"-threNode" );
        enod = m_thrEnode;
        m_cv->setEnode( enod );
    }
    else if( enod != m_thrEnode ) m_thrEnode = enod;// Connected to external eNode

    m_ePinA.setEnode( m_thrEnode );  // Set eNode to internal eResistors ePins
    m_ePinB.setEnode( m_thrEnode );

    enod = m_Gnd->getEnode();       // Gnd Pin
    m_ePinC.setEnode( enod );  // Set eNode to internal eResistors ePins
    m_ePinD.setEnode( enod );
}

void Lm555::stamp()
{
    m_resA.setRes( 5000 );
    m_resB.setRes( 10000 );
    m_resD.setRes( 1 );

    for( int i=0; i<8; ++i )
    {
        if( (i == 2) || (i == 6) ) continue; // Output or Discharge
        if( m_pin[i]->connector() ) m_pin[i]->changeCallBack( this );//  ->getEnode()->addToNoLinList(this);
    }
    Simulator::self()->addEvent( 1, NULL );
}

void Lm555::voltChanged()
{
    if( !m_Gnd->connector() || !m_Vcc->connector() ) return;

    bool changed = false;
    double voltPos = m_Vcc->getVolt();
    double voltNeg = m_Gnd->getVolt();

    if( fabs(voltNeg-m_voltNeg) > 1e-3 )
    {
        m_output->setOutLowV( voltNeg );
        m_voltNeg = voltNeg;
        changed = true;
    }
    if( fabs(voltPos-m_voltPos) > 1e-3 )
    {
        double voltHight = voltPos - 1.7;
        if( voltHight < m_voltNeg ) voltHight = m_voltNeg;
        m_output->setOutHighV( voltHight );

        m_voltPos = voltPos;
        changed = true;
    }

    double refThre = m_thrEnode->getVolt()-m_voltNeg; // Voltage at threshold node.
    double refTrig = refThre/2;

    bool outState = m_outState;

    if     ( m_Reset->getVolt() < (m_voltNeg+0.7) ) outState = false; // Reset
    else if( m_threshold->getVolt() > refThre )     outState = false; // Threshold
    else if( refTrig > m_trigger->getVolt() )       outState = true;  // Trigger

    if( outState != m_outState)
    {
        m_outState = outState;
        changed = true;
    }
    if( changed ) Simulator::self()->addEvent( m_propDelay, this );
}

void Lm555::runEvent()
{
    m_output->setOutState( m_outState );
    m_resD.setRes( m_outState ? high_imp : 1 );
}

void Lm555::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area, 1, 1);
    
    p->setPen( QColor( 170, 170, 150 ) );
    p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
}

#include "moc_lm555.cpp"
