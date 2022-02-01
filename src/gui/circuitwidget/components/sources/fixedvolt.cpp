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

#include <QToolButton>
#include <QPainter>

#include "fixedvolt.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* FixedVolt::construct( QObject* parent, QString type, QString id )
{ return new FixedVolt( parent, type, id ); }

LibraryItem* FixedVolt::libraryItem()
{
    return new LibraryItem(
        tr( "Fixed Volt." ),
        tr( "Sources" ),
        "voltage.png",
        "Fixed Voltage",
        FixedVolt::construct );
}

FixedVolt::FixedVolt( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , eElement( id )
{
    m_area = QRect( -10, -10, 20, 20 );

    m_graphical = true;
    m_changed   = false;

    m_pin.resize(1);
    m_pin[0] = m_outpin = new IoPin( 0, QPoint(16,0), id+"-outnod", 0, this, source );

    setValLabelPos(-16, 8 , 0 ); // x, y, rot 
    setLabelPos(-64,-24 );

    m_button = new QToolButton();
    m_button->setMaximumSize( 16,16 );
    m_button->setGeometry(-20,-16,16,16);
    m_button->setCheckable( true );

    m_proxy = Circuit::self()->addWidget( m_button );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-32, -8) );

    setVolt( 5.0 );
    
    Simulator::self()->addToUpdateList( this );

    connect( m_button, SIGNAL( clicked() ),
             this,     SLOT  ( onbuttonclicked() ), Qt::UniqueConnection );

    addPropGroup( { "Hidden", {
new BoolProp<FixedVolt>( "Out", "","", this, &FixedVolt::out, &FixedVolt::setOut ),
    }} );
    addPropGroup( { tr("Main"), {
new DoubProp<FixedVolt>( "Voltage", tr("Voltage"),"V", this, &FixedVolt::volt, &FixedVolt::setVolt )
    }} );
}
FixedVolt::~FixedVolt(){}

void FixedVolt::stamp()
{
    if( !Simulator::self()->isPaused() ) m_changed = true;
}

void FixedVolt::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;
    m_outpin->sheduleState( m_button->isChecked(), 0 );
    update();
}

void FixedVolt::onbuttonclicked()
{
    m_changed = true;
    update();
}

void FixedVolt::updateOutput()
{
    m_outpin->setOutHighV( m_voltage );
    m_changed = true;
}

bool FixedVolt::out() { return m_button->isChecked(); }

void FixedVolt::setOut( bool out )
{
    m_button->setChecked( out );
    onbuttonclicked();
}

void FixedVolt::setVolt( double v )
{
    m_voltage = v;
    m_outpin->setOutHighV( m_voltage );
}

void FixedVolt::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    if( m_button->isChecked() ) p->setBrush( QColor( 255, 166, 0 ) );
    else                        p->setBrush( QColor( 230, 230, 255 ) );

    p->drawRoundedRect( QRectF( -8, -8, 16, 16 ), 2, 2);
}
#include "moc_fixedvolt.cpp"
