/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "mux.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "connector.h"
#include "iopin.h"

#include "boolprop.h"
#include "intprop.h"

Component* Mux::construct( QObject* parent, QString type, QString id )
{ return new Mux( parent, type, id ); }

LibraryItem* Mux::libraryItem()
{
    return new LibraryItem(
        tr( "Mux" ),
         "Converters" ,
        "mux.png",
        "Mux",
        Mux::construct );
}

Mux::Mux( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 10;
    m_tristate = true;

    setNumInps( 8,"D" );
    
    init({          // Inputs:
            "ID03S0",
            "ID02S1",
            "ID01S2",
                   // Outputs:
            "OR02Y",
            "OR03!Y"
        });

    m_inPin[9]->setLength( 6 );
    m_inPin[10]->setLength( 4 );

    createOePin( "IU03OE ", id+"-in11");

    m_addrBits = 0;
    setAddrBits( 3 );

    addPropGroup( { tr("Main"), {
new IntProp<Mux>(  "Address_Bits" , tr("Address Size") ,"_bits", this, &Mux::addrBits,   &Mux::setAddrBits)
    }, groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({new BoolProp<Mux>( "Invert_Inputs", tr("Invert Inputs"),"", this, &Mux::invertInps, &Mux::setInvertInps, propNoCopy )})
                    +IoComponent::outputProps(),0 } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps(),0 } );
}
Mux::~Mux(){}

void Mux::stamp()
{
    for( int i=0; i<11; ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp();
}

void Mux::voltChanged()
{
    LogicComponent::updateOutEnabled();

    int address = 0;

    for( int i=8; i<11; ++i )
        if( m_inPin[i]->getInpState() ) address += pow( 2, i-8 );

    bool out = m_inPin[address]->getInpState();
    m_nextOutVal = out? 1:2;
    sheduleOutPuts( this );
}

void Mux::setAddrBits( int bits )
{
    if( m_addrBits == bits ) return;

    if     ( bits < 1 ) bits = 1;
    else if( bits > 3 ) bits = 3;

    int channels = pow( 2, bits );
    m_addrBits = bits;

    m_height = channels+2;
    int bit0 = 8;
    int w = m_width*8/2;
    int h = m_height*8/2;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    for( int i=0; i<3; ++i )
    {
        Pin* pin = m_inPin[bit0+i];
        if( i < bits )
        {
            pin->setVisible( true );
            pin->setY( h+8 );
        }else{
            pin->removeConnector();
            pin->setVisible( false );
        }
        if( i < 2 ) m_outPin[i]->setY( -h+i*8+16 ); // Outputs
    }
    for( int i=0; i<8; ++i )
    {
        Pin* pin = m_inPin[i];
        if( i < channels )
        {
            pin->setVisible( true );
            pin->setY( i*8-(bits+bits/3)*8 );
        }else{
            pin->removeConnector();
            pin->setVisible( false );
    }   }
    m_oePin->setY( -h-8 ); // OE

    m_area = QRect(-w-1,-h-8-1, w*2+2, h*2+16+2 );
    Circuit::self()->update();
}

QPainterPath Mux::shape() const
{
    QPainterPath path;
    
    int w = m_width*8/2;
    int h = m_height*8/2;

    QVector<QPointF> points;
    
    points << QPointF(-w,-h-6 )
           << QPointF(-w, h+6 )
           << QPointF( w, h-2 )
           << QPointF( w,-h+2 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Mux::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    int w = m_width*8/2;
    int h = m_height*8/2;

    QPointF points[4] = {
        QPointF(-w,-h-6 ),
        QPointF(-w, h+6 ),
        QPointF( w, h-2 ),
        QPointF( w,-h+2 )};

    p->drawPolygon(points, 4);
}
