/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QGraphicsProxyWidget>

#include "push.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simulator.h"
#include "pin.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

#define tr(str) simulideTr("Push",str)

Component* Push::construct( QString type, QString id )
{ return new Push( type, id ); }

LibraryItem* Push::libraryItem()
{
    return new LibraryItem(
        tr("Push"),
        "Switches",
        "push.png",
        "Push",
        Push::construct);
}

Push::Push( QString type, QString id )
    : PushBase( type, id )
{
    m_area = QRectF(-11,-9, 22, 11 );
    m_proxy->setPos(-8, 4 );

    SetupSwitches( 1, 1 );

    addPropGroup( { tr("Main"), {
        new BoolProp<Push>("Norm_Close", tr("Normally Closed"), ""
                          , this, &Push::nClose, &Push::setNClose ),

        new BoolProp<Push>("DT", tr("Double Throw"), ""
                          , this, &Push::dt, &Push::setDt, propNoCopy ),

        new IntProp <Push>("Poles", tr("Poles"), ""
                          , this, &Push::poles, &Push::setPoles, propNoCopy,"uint" ),

        new StrProp <Push>("Key", tr("Key"), ""
                          , this, &Push::key, &Push::setKey ),
    }, 0} );
}
Push::~Push(){}

void Push::stamp()
{
    if( m_ButHidden ) return;

    setSwitch( m_nClose );
}

void Push::SetupSwitches( int poles, int throws )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    m_area = QRectF( -12, 8-16*poles, 24, 16*poles-4 );

    for( uint i=0; i<m_switches.size(); i++ ) delete m_switches[i];
    for( uint i=0; i<m_pin.size();  i++ ) deletePin( m_pin[i] );

    m_numPoles = poles;
    m_numthrows = throws;

    m_switches.resize( poles*throws );
    m_pin.resize( poles*throws*2 );

    int pinN = 0;
    for( int i=0; i<poles; i++ )              // Create Resistors
    {
        Pin* pin;

        for( int j=0; j<throws; j++ )
        {
            int tN = i*throws+j;
            QString reid = m_id+"-switch"+QString::number(tN);
            m_switches[ tN ] = new eResistor( reid );

            QPoint pinpos = QPoint(-16,-4*m_pin0-16*i-8*j);
            pin = new Pin( 180, pinpos, m_id+"-pinP"+QString::number(pinN), 0, this);
            pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top
            m_pin[pinN] = pin;
            m_switches[ tN ]->setEpin( 0, pin );
            pinN++;

            pinpos = QPoint( 16,-4*m_pin0-16*i-8*j);
            pin = new Pin( 0, pinpos, reid+"pinN", 1, this);
            pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top
            m_pin[pinN] = pin;
            m_switches[ tN ]->setEpin( 1, pin );
            pinN++;
        }
    }
    Circuit::self()->update();
}

void Push::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );
    
    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    for( int i=0; i<m_numPoles; i++ )                           // Draw Switches
    {
        int offset = 16*i;
        if( m_closed ) p->drawLine(-9,-2-offset, 9,-2-offset );
        else           p->drawLine(-9,-6-offset, 9,-6-offset );
    }
    if( m_numPoles > 1 )
    {
        pen.setStyle(Qt::DashLine);
        pen.setWidth(1);
        p->setPen(pen);
        p->drawLine(-0, 4-4*m_pin0, 0,-3*m_pin0-16*m_numPoles+4 );
    }
    Component::paintSelected( p );
}
