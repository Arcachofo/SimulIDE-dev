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

#include <math.h>

#include "probe.h"
#include "connector.h"
#include "connectorline.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "label.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* Probe::construct( QObject* parent, QString type, QString id )
{ return new Probe( parent, type, id ); }

LibraryItem* Probe::libraryItem()
{
    return new LibraryItem(
        tr( "Probe" ),
        tr( "Meters" ),
        "probe.png",
        "Probe",
        Probe::construct );
}

Probe::Probe( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_graphical = true;
    
    m_area = QRect( -8, -8, 16, 16 );
    m_voltTrig = 2.5;

    m_pin.resize(1); // Create Input Pin
    m_pin[0] = m_inputPin = new IoPin( 180, QPoint(-22,0), id+"-inpin", 0, this, input);
    m_inputPin->setLength( 20 );
    m_inputPin->setBoundingRect( QRect(-2, -2, 6, 4) );
    m_inputPin->setImp( 1e9 );

    setValLabelPos( 16, 0, 45 ); // x, y, rot
    setShowVolt( true );
    setLabelPos( 16,-16, 45 );
    setRotation( rotation() - 45 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new BoolProp<Probe>( "ShowVolt" , tr("Show Voltage"), "" , this, &Probe::showVolt,  &Probe::setShowVolt ),
new DoubProp<Probe>( "Threshold", tr("Threshold")   , "V", this, &Probe::threshold, &Probe::setThreshold )
    } } );
}
Probe::~Probe(){}

void Probe::updateStep()
{
    if( !Simulator::self()->isRunning() ) { setVolt( 0.0 ); return; }

    if( m_inputPin->isConnected() )// Voltage from connected pin
    {
         setVolt( m_inputPin->getVolt() );
         return;
    }
    QList<QGraphicsItem*> list = m_inputPin->collidingItems(); // Voltage from connector or Pin behind inputPin
    if( list.isEmpty() ) { setVolt( 0.0 ); return; }

    for( QGraphicsItem* it : list )
    {
        if( it->type() == UserType+3 )                    // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );
            setVolt(pin->getVolt() );
            break;
        }else if( it->type() == UserType+2 )        // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );
            Connector* con = line->connector();
            setVolt( con->getVolt() );
            break;
}   }   }

void Probe::setShowVolt( bool show )
{
    m_showVolt = show;
    m_valLabel->setVisible( show );
}

void Probe::setVolt( double volt )
{
    if( m_voltIn == volt ) return;
    m_voltIn = volt;

    if( !m_showVolt ) return;
    if( fabs(volt) < 0.01 ) volt = 0;
    int dispVolt = int( volt*100+0.5 );
    
    m_valLabel->setPlainText( QString("%1 V").arg(double(dispVolt)/100) );
    update();       // Repaint
}

QPainterPath Probe::shape() const
{
    QPainterPath path;
    path.addEllipse( m_area );
    return path;
}
void Probe::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    if      ( m_voltIn > m_voltTrig)  p->setBrush( QColor( 255, 166, 0 ) );
    else if ( m_voltIn < -m_voltTrig) p->setBrush( QColor( 0, 100, 255 ) );
    else                              p->setBrush( QColor( 230, 230, 255 ) );

    p->drawEllipse( m_area );
}
