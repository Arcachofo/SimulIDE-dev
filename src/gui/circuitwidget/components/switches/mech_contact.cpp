/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mech_contact.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simulator.h"
#include "e-node.h"

MechContact::MechContact( QObject* parent, QString type, QString id )
           : Component( parent, type, id )
           , eElement(  id )
{
    m_numthrows = 0;
    m_numPoles  = 0;
    m_pin0      = 0;

    m_nClose = false;
    m_closed = false;
    m_ButHidden = false;

    Simulator::self()->addToUpdateList( this );
}
MechContact::~MechContact(){}

void MechContact::stamp()
{
    if( m_ButHidden ) return;
    for( int i=0; i<m_numPoles; ++i )
    {
        eNode* enode = m_pin[m_pin0/2+i*(1+m_numthrows)]->getEnode();

        int epinN = m_pin0+i*m_numthrows*2;
        m_ePin[ epinN ]->setEnode( enode );

        if( m_numthrows > 1 ) m_ePin[ epinN+2 ]->setEnode( enode );
    }
    for( uint i=m_pin0; i<m_ePin.size(); ++i )
    {
        eNode* enode = m_ePin[i]->getEnode();
        if( enode ) enode->setSwitched( true );
    }
    setSwitch( m_nClose );
}

void MechContact::setSwitch( bool closed )
{
    m_changed = true;
    m_closed = closed;

    for( int i=0; i<m_numPoles; i++ )
    {
        int switchN = i*m_numthrows;

        if( closed ) m_switches[ switchN ]->setAdmit( 1e3 );
        else         m_switches[ switchN ]->setAdmit( 0 );

        if( m_numthrows == 2 )
        {
            switchN++;
            if( closed ) m_switches[ switchN ]->setAdmit( 0 );
            else         m_switches[ switchN ]->setAdmit( 1e3 );
}   }   }

void MechContact::remove()
{
    for( eResistor* res : m_switches ) delete res;
    Component::remove();
}

void MechContact::SetupButton()
{
    setNumEpins(2);
    m_switches.resize( 1 );
    m_switches[ 0 ] = new eResistor( "res0" );
    m_switches[ 0 ]->setEpin( 0, m_ePin[0] );
    m_switches[ 0 ]->setEpin( 1, m_ePin[1] );
    m_numPoles = 1;
    m_numthrows = 1;
    m_ButHidden = true;
}

void MechContact::SetupSwitches( int poles, int throws )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    if( m_pin0  == 0 ) m_area = QRectF( -12, 8-16*poles,   24, 16*poles-4 );     // Switches
    else               m_area = QRectF( -12,-8-16*poles-4, 24, 8+16*poles+8+4 ); // Relays
    int start = m_pin0/2;

    for( uint i=0; i<m_switches.size(); i++ ) delete m_switches[i];
        
    for( int i=0; i<m_numPoles; i++ )
    {
        int epinN = m_pin0+i*m_numthrows*2;
        delete m_ePin[ epinN ];
        if( m_numthrows > 1 ) delete m_ePin[ epinN+2 ];
    }
    for( uint i=start; i<m_pin.size(); i++ )
    {
        m_pin[i]->removeConnector();
        m_signalPin.removeAll( m_pin[i] );
        delete m_pin[i];
    }
    m_numPoles = poles;
    m_numthrows = throws;

    m_switches.resize( poles*throws );
    m_pin.resize( start+poles+poles*throws );
    m_ePin.resize( m_pin0+2*poles*throws );

    int cont = 0;
    for( int i=0; i<poles; i++ )              // Create Resistors
    {
        Pin* pin;
        int pinN = start+cont;
        int ePinN = m_pin0+cont;

        QPoint pinpos = QPoint(-16,-4*m_pin0-16*i );
        pin = new Pin( 180, pinpos, m_id+"-pinP"+QString::number(pinN), 0, this);
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top
        m_pin[pinN] = pin;

        for( int j=0; j<throws; j++ )
        {
            cont++;
            int tN = i*throws+j;

            QString reid = m_id+"-switch"+QString::number(tN);
            m_switches[ tN ] = new eResistor( reid );

            ePinN = m_pin0+tN*2;
            m_ePin[ ePinN ] = new ePin( reid+"pinP", 1 );

            pinpos = QPoint( 16,-4*m_pin0-16*i-8*j);
            pin = new Pin( 0, pinpos, reid+"pinN", 1, this);
            pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top

            m_pin[ start+cont ] = pin;
            m_ePin[ ePinN+1 ]   = pin;

            m_switches[ tN ]->setEpin( 0, m_ePin[ePinN] );
            m_switches[ tN ]->setEpin( 1, pin );
        }
        cont++;
    }
    Circuit::self()->update();
}

void MechContact::setPoles( int poles )
{
    if( poles < 1 ) poles = 1;

    if( poles != m_numPoles )
        SetupSwitches( poles, m_numthrows );
}

void MechContact::setDt( bool dt )
{
    int throws = 1;
    if( dt ) throws = 2;

    if( throws != m_numthrows )
        SetupSwitches( m_numPoles, throws );
}

void MechContact::setNClose( bool nc )
{
    m_nClose = nc;
    setSwitch( m_nClose );
    update();
}

void MechContact::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_ButHidden ) return;

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    for( int i=0; i<m_numPoles; i++ )                           // Draw Switches
    {
        int offset = 16*i-16+4*m_pin0;

        if( m_closed )                                          // switch is closed
            p->drawLine(-10, -16-offset, 10, -18-offset );
        else                                                    // Switch is oppened
            p->drawLine(-10.5, -16-offset, 8, -24-offset );
    }
    if( m_numPoles > 1 )
    {
        pen.setStyle( Qt::DashLine );
        pen.setWidth( 1 );
        p->setPen( pen );
        p->drawLine( 0, 4-4*m_pin0, 0, -3*m_pin0-16*m_numPoles+8 );
}   }

