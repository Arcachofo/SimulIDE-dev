/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>

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
        "Meters",
        "probe.png",
        "Probe",
        Probe::construct );
}

Probe::Probe( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    setZValue( 200 );
    m_area = QRect(-12,-8, 20, 16 );
    m_graphical = true;
    m_voltTrig = 2.5;
    m_voltIn = 0;

    m_pin.resize(1); // Create Input Pin
    m_pin[0] = m_inputPin = new IoPin( 180, QPoint(-22,0), id+"-inpin", 0, this, input);
    m_inputPin->setLength( 14 );
    m_inputPin->setBoundingRect( QRect(-1, -1, 2, 2) );
    m_inputPin->setImp( 1e9 );

    setValLabelPos( 16, 0, 45 ); // x, y, rot
    setShowVal( true );
    setLabelPos( 16,-16, 45 );
    setRotation( rotation() - 45 );
    m_voltIn = -1; // Force update
    setVolt( 0 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new BoolProp<Probe>( "ShowVolt" , tr("Show Voltage"), "" , this, &Probe::showVal,   &Probe::setShowVal ),
new DoubProp<Probe>( "Threshold", tr("Threshold")   , "V", this, &Probe::threshold, &Probe::setThreshold )
    }, groupNoCopy } );
}
Probe::~Probe(){}

void Probe::updateStep()
{
    if( !Simulator::self()->isRunning() ) { setVolt( 0.0 ); return; }

    if( m_inputPin->isConnected() )// Voltage from connected pin
    {
         setVolt( m_inputPin->getVoltage() );
         return;
    }
    QList<QGraphicsItem*> list = m_inputPin->collidingItems(); // Voltage from connector or Pin behind inputPin
    if( list.isEmpty() ) { setVolt( 0.0 ); return; }

    for( QGraphicsItem* it : list )
    {
        if( it->type() == UserType+3 )                    // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );
            setVolt( pin->getVoltage() );
            break;
        }else if( it->type() == UserType+2 )        // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );
            Connector* con = line->connector();
            setVolt( con->getVoltage() );
            break;
}   }   }

void Probe::setVolt( double volt )
{
    if( m_voltIn == volt ) return;
    m_voltIn = volt;
    update();       // Repaint

    if( !m_showVal ) return;
    if( qFabs(volt) < 0.01 ) volt = 0;
    
    setValLabelText( QString("%1 V").arg(float(int( volt*100+0.5 ))/100) );
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

    p->drawEllipse( QRect(-8,-8, 16, 16 ) );
}
