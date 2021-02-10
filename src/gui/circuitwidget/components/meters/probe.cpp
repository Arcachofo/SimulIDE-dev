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

#include "probe.h"
#include "connector.h"
#include "connectorline.h"
#include "e-source.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "pin.h"

#include <math.h>

static const char* Probe_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Show volt"),
    QT_TRANSLATE_NOOP("App::Property","Threshold")
};

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
    Q_UNUSED( Probe_properties );

    m_graphical = true;
    
    m_area = QRect( -8, -8, 16, 16 );
    m_readPin = 0l;
    m_readConn = 0l;
    m_voltTrig = 2.5;
    m_plotterColor = QColor( 255, 255, 255 );

    // Create Input Pin
    m_ePin.resize(1);
    m_pin.resize(1);
    QString nodid = id;
    nodid.append(QString("-inpin"));
    QPoint nodpos = QPoint(-22,0);
    m_inputpin = new Pin( 180, nodpos, nodid, 0, this);
    m_inputpin->setLength( 20 );
    m_inputpin->setBoundingRect( QRect(-2, -2, 6, 4) );
    m_pin[0] = m_inputpin;
    
    nodid.append( QString("-eSource") );
    m_inSource = new eSource( nodid, m_inputpin, input );
    m_inSource->setImp( 1e9 );

    setRotation( rotation() - 45 );
    
    m_unit = " V";
    m_valLabel->setDefaultTextColor( Qt::darkRed );
    m_valLabel->setPlainText( "0" );
    setValLabelPos( 16, 0 , 45 ); // x, y, rot 
    setVolt( 0 );
    setShowVal( true );
    
    setLabelPos( 16, -16 , 45 );

    Simulator::self()->addToUpdateList( this );
}
Probe::~Probe()
{
    delete m_inSource;
}

QList<propGroup_t> Probe::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Threshold", tr("Threshold"),"V"} );
    return {mainGroup};
}

void Probe::updateStep()
{
    m_readPin = 0l;
    m_readConn = 0l;
    
    if( !Simulator::self()->isRunning() )
    {
        setVolt( 0.0 );
        return;
    }

    if( m_inputpin->isConnected() )// Voltage from connected pin
    {
         setVolt( m_inputpin->getVolt() );
         return;
    }

    // Voltage from connector or Pin behind inputPin
    QList<QGraphicsItem*> list = m_inputpin->collidingItems();

    if( list.isEmpty() )
    {
        setVolt( 0.0 );
        return;
    }
    for( QGraphicsItem* it : list )
    {
        if( it->type() == 65536 )                           // Component
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );

            Connector* con = line->connector();

            if( con->objectName().startsWith("Connector") ) // Connector found
            {
                setVolt( con->getVolt() );
                m_readConn = con;
                break;
            }
        }
        else if( it->type() == 65536+3 )                    // Pin found
        {
            m_readPin =  qgraphicsitem_cast<Pin *>( it );
            setVolt( m_readPin->getVolt() );
            break;
        }
    }
}

void Probe::setVolt( double volt )
{
    if( m_voltIn == volt ) return;

    m_voltIn = volt;

    if( fabs(volt) < 0.01 ) volt = 0;
    int dispVolt = int( volt*100+0.5 );
    
    m_valLabel->setPlainText( QString("%1 V").arg(double(dispVolt)/100) );

    update();       // Repaint
}

double Probe::getVolt()
{
    double volt = 0;
    if     ( m_inputpin->isConnected() ) volt = m_inputpin->getVolt();
    else if( m_readConn != 0l )          volt = m_readConn->getVolt();
    else if( m_readPin != 0l )           volt = m_readPin->getVolt();
    return volt;
}

void Probe::remove()
{
    if( m_inputpin->isConnected() ) m_inputpin->connector()->remove();

    emit removed( this );
    
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
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

#include "moc_probe.cpp"

