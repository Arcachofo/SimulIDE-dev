/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "bus.h"
#include "connector.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "intprop.h"

Component* Bus::construct( QObject* parent, QString type, QString id )
{ return new Bus( parent, type, id ); }

LibraryItem* Bus::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("Bus", "Bus"),
        "Connectors",
        "bus.png",
        "Bus",
        Bus::construct );
}

Bus::Bus( QObject* parent, QString type, QString id )
   : Component( parent, type, id )
   , eElement( id )
{
    m_busPin1 = new Pin( 270, QPoint( 0, 0 ), m_id+"-busPinI", 1, this );
    m_busPin1->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    m_busPin1->setLength( 1 );
    m_busPin1->setIsBus( true );

    m_numLines = 0;
    m_startBit = 0;
    setNumLines( 8 );       // Create Input Pins

    m_ePin[0] = m_pin[0] = m_busPin0 = new Pin( 90, QPoint( 0, 0 ), m_id+"-ePin0", 1, this );
    m_busPin0->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    m_busPin0->setLength( 1 );
    m_busPin0->setIsBus( true );

    addPropGroup( { tr("Main"), {
new IntProp<Bus>("Num_Bits" , tr("Size")     ,"_Bits", this, &Bus::numLines, &Bus::setNumLines,0,"uint" ),
new IntProp<Bus>("Start_Bit", tr("Start Bit"),""     , this, &Bus::startBit, &Bus::setStartBit,0,"uint" )
    },groupNoCopy} );
}
Bus::~Bus(){}

void Bus::registerEnode( eNode* enode, int n )
{
    if( m_busPin0->conPin() ) m_busPin0->registerPinsW( enode, n );
    if( m_busPin1->conPin() ) m_busPin1->registerPinsW( enode, n );

    int i = n + 1 - m_startBit;
    if( i < 0 ) return;
    if( i > m_numLines ) return;

    if( m_pin[i]->conPin() ) m_pin[i]->registerPinsW( enode, -1 );
}

void Bus::setNumLines( int lines )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    if( lines == m_numLines ) return;
    if( lines < 1 ) return;

    for( int i=1; i<=m_numLines; i++ )
    {
        Pin* pin = m_pin[i];
        if( pin->connector() ) pin->connector()->remove();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        m_signalPin.removeAll( pin );
        delete pin;
    }
    m_numLines = lines;

    m_pin.resize( lines+2 );
    m_ePin.resize( lines+2 );
    
    for( int i=1; i<=lines; i++ )
    {
        QString pinId = m_id+"-ePin"+QString::number(i);
        Pin* pin = new Pin( 180, QPoint(-8, -8*lines+i*8 ), pinId, m_startBit+i-1, this );

        pin->setFontSize( 4 );
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        pin->setLabelText( " "+QString::number( m_startBit+i-1 )+" " );
        m_pin[i]  = pin;
        m_ePin[i] = pin;
    }
    m_busPin1->setPos( QPoint( 0 ,-lines*8+8 ) );
    m_busPin1->isMoved();
    m_pin[ lines+1 ]  = m_busPin1;
    m_ePin[ lines+1 ] = m_busPin1;

    m_height = lines-1;
    m_area = QRect( -3,-m_height*8-2, 5, m_height*8+4 );
    Circuit::self()->update();
}

void Bus::setStartBit( int bit )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    if( bit < 0 ) bit = 0;
    m_startBit = bit;

    for( int i=1; i<=m_numLines; i++ )
    {
        m_pin[i]->setIndex( m_startBit+i-1 );
        m_pin[i]->setLabelText( " "+QString::number( m_startBit+i-1 ) );
    }
}

void Bus::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawRect( QRect( 0, -m_height*8, 0, m_height*8 ) );
              //QRect( -2, -m_height*8-4, 2, m_height*8+8 );
}
