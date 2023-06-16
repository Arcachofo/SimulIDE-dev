/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <math.h>

#include "relay.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "e-node.h"

#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* Relay::construct( QObject* parent, QString type, QString id )
{ return new Relay( parent, type, id ); }

LibraryItem* Relay::libraryItem()
{
    return new LibraryItem(
        tr( "Relay (all)" ),
        "Switches",
        "relay-spst.png",
        "RelaySPST",
        Relay::construct);
}

Relay::Relay( QObject* parent, QString type, QString id )
     : MechContact( parent, type, id )
{
    m_ePin.resize(4);
    m_pin.resize(2);

    m_pin0 = 4;

    m_inductor = (Inductor*)Circuit::self()->createItem( "Inductor", m_id+"-inductor" );//new Inductor( this, "Inductor", m_id+"-inductor" );
    m_inductor->setParentItem( this );
    m_inductor->moveTo( QPointF( 0, 0 ) );
    m_inductor->setValue( 0.1 );  // 100 mH
    m_inductor->setResist( 100 );
    m_inductor->setShowVal( false );

    m_pin[0] = m_inductor->getPin( 0 );
    m_pin[1] = m_inductor->getPin( 1 );

    m_trigCurrent = 0.02;
    m_relCurrent  = 0.01;

    setValLabelPos(-16, 6, 0);
    setLabelPos(-16, 8, 0);

    SetupSwitches( 1, 1 );

    addPropGroup( { tr("Main"), {
new BoolProp<Relay>( "Norm_Close", tr("Normally Closed"),""      , this, &Relay::nClose, &Relay::setNClose ),
new BoolProp<Relay>( "DT"        , tr("Double Throw")   ,""      , this, &Relay::dt,     &Relay::setDt, propNoCopy ),
new IntProp <Relay>( "Poles"     , tr("Poles")          ,"_Poles", this, &Relay::poles,  &Relay::setPoles, propNoCopy,"uint" ),
    }, 0} );
    addPropGroup( { tr("Electric"), {
new DoubProp<Relay>( "IOn" , tr("IOn") ,"A", this, &Relay::iTrig, &Relay::setITrig),
new DoubProp<Relay>( "IOff", tr("IOff"),"A", this, &Relay::iRel,  &Relay::setIRel )
    }, 0} );
    addPropGroup( { tr("Coil"), {
new DoubProp<Inductor>( "Inductance", tr("Inductance"),"H", m_inductor, &Inductor::value , &Inductor::setValue ),
new DoubProp<Inductor>( "Rcoil"     , tr("Resistance"),"Ω", m_inductor, &Inductor::resist, &Inductor::setResist),
new IntProp <Inductor>( "AutoStep"  , tr("Auto Step") , "_Steps", m_inductor, &Inductor::autoStep, &Inductor::setAutoStep,0,"uint" )
    }, 0} );
}
Relay::~Relay(){}

void Relay::initialize()
{
    m_relayOn = false;

    if( Simulator::self()->isRunning() )
        m_internalEnode = new eNode( m_id+"-internaleNode" );
}

void Relay::stamp()
{
    MechContact::stamp();

    m_inductor->getPin( 0 )->changeCallBack( this );
    m_inductor->getPin( 1 )->changeCallBack( this );
}

void Relay::voltChanged()
{
    double indCurr = fabs( m_inductor->indCurrent() );
    bool relayOn;

    if( m_relayOn ) relayOn = ( indCurr > m_relCurrent );
    else            relayOn = ( indCurr > m_trigCurrent );
    m_relayOn = relayOn;

    if( m_nClose ) relayOn = !relayOn;
    if( relayOn != m_closed ) setSwitch( relayOn );
}

void Relay::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->drawRect( m_area );

    MechContact::paint( p, option, widget );
}
