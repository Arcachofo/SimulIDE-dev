/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <QDial>
#include <QPainter>
#include <QGraphicsProxyWidget>

#include "potentiometer.h"
#include "propdialog.h"
#include "connector.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "doubleprop.h"

Component* Potentiometer::construct( QObject* parent, QString type, QString id )
{ return new Potentiometer( parent, type, id ); }

LibraryItem* Potentiometer::libraryItem()
{
    return new LibraryItem(
        tr( "Potentiometer" ),
        tr( "Resistors" ),
        "potentiometer.png",
        "Potentiometer",
        Potentiometer::construct );
}

Potentiometer::Potentiometer( QObject* parent, QString type, QString id )
             : Component( parent, type, id )
             , eElement( (id+"-eElement") )
             , m_pinA( 180, QPoint(-16,0 ), id+"-PinA", 0, this )
             , m_pinM( 270, QPoint( 0,16 ), id+"-PinM", 0, this )
             , m_pinB(   0, QPoint( 16,0 ), id+"-PinB", 0, this )
             , m_ePinA( (id+"-ePinA"), 1 )
             , m_ePinB( (id+"-ePinB"), 1 )
             , m_resA(  (id+"-resA") )
             , m_resB(  (id+"-resB") )
{
    m_graphical = true;
    m_midEnode = NULL;
    m_area = QRectF( -12, -4.5, 24, 12.5 );

    m_pin.resize(3);
    m_pin[0] = &m_pinA;
    m_pin[1] = &m_pinM;
    m_pin[2] = &m_pinB;
    
    m_dialW.setupWidget();
    m_dialW.setFixedSize( 24, 24 );
    m_dialW.dial->setMinimum(0);
    m_dialW.dial->setMaximum(1000);
    m_dialW.dial->setValue(500);
    m_dialW.dial->setSingleStep(25);
    
    m_proxy = Circuit::self()->addWidget( &m_dialW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint( -12, -24-5) );
    //m_proxy->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true );

    m_dial = m_dialW.dial;
    
    m_resA.setEpin( 0, &m_pinA );
    m_resA.setEpin( 1, &m_ePinA );
    m_resB.setEpin( 1, &m_pinB );
    m_resB.setEpin( 0, &m_ePinB );

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);
    
    Simulator::self()->addToUpdateList( this );

    connect( m_dial, SIGNAL(valueChanged(int)),
             this,   SLOT  (resChanged(int)), Qt::UniqueConnection );

    addPropGroup( { tr("Main"), {
new DoubProp<Potentiometer>( "Resistance", tr("Resistance")   ,"Ω", this, &Potentiometer::getRes, &Potentiometer::setRes ),
new DoubProp<Potentiometer>( "Value_Ohm" , tr("Current Value"),"Ω", this, &Potentiometer::getVal, &Potentiometer::setVal )
    } } );

    m_res1 = 0;
    setShowProp("Resistance");
    setPropStr( "Resistance", "1000 Ω" );
    resChanged( 500 );
}
Potentiometer::~Potentiometer() {}

void Potentiometer::initialize()
{
    if( !Simulator::self()->isRunning() ) return;

    m_midEnode = m_pinM.getEnode();  // Get eNode from middle Pin
    if( !m_midEnode ) m_midEnode = new eNode( m_id+"-mideNode" ); // Not connected: Create mid eNode
}

void Potentiometer::stamp()
{
    m_pinM.setEnode( m_midEnode );
    m_ePinA.setEnode( m_midEnode );  // Set eNode to internal eResistors ePins
    m_ePinB.setEnode( m_midEnode );

    m_changed = true;
    updateStep();
}

void Potentiometer::registerEnode( eNode* enode, int ) // called by m_pin[0]
{
    m_ePinA.setEnode( enode );  // Set eNode to internal eResistors ePins
    m_ePinB.setEnode( enode );
}

void Potentiometer::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    double res1 = double( m_resist*m_dial->value()/1000 );
    double res2 = m_resist-res1;

    if( res1 < 1e-6 ){
        res1 = 1e-3;
        res2 = m_resist-res1;
    }
    if( res2 < 1e-6 ){
        res2 = 1e-6;
        res1 = m_resist-res2;
    }
    m_resA.setRes( res1 );
    m_resB.setRes( res2 );

    if( m_propDialog ) m_propDialog->updtValues();
}

void Potentiometer::resChanged( int res ) // Called when dial is rotated
{
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

double Potentiometer::getVal() { return m_resist*m_dial->value()/1000; }

void Potentiometer::setVal( double val )
{
    if( val > m_resist ) val = m_resist;
    else if( val < 1e-12 ) val = 1e-12;
    m_dial->setValue( val*1000/m_resist );
    m_res1 = val;
    m_changed = true;
    updateStep();
}

void Potentiometer::setRes( double res ) // Called when property resistance is changed
{
    if( res < 1e-12 ) res = 1e-12;
    m_resist = res;
    setVal( m_res1 );
}

void Potentiometer::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );
    p->drawRect( -10.5, -4, 21, 8 );
    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine( 0, 6, -3, 9 );
    p->drawLine( 0, 6,  3, 9 );
}

#include "moc_potentiometer.cpp"
