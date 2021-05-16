/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "mux_analog.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "e-resistor.h"
#include "iopin.h"

static const char* MuxAnalog_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Address Bits")
};

Component* MuxAnalog::construct( QObject* parent, QString type, QString id )
{
        return new MuxAnalog( parent, type, id );
}

LibraryItem* MuxAnalog::libraryItem()
{
    return new LibraryItem(
        tr( "Analog Mux" ),
        tr( "Active" ),
        "1to3-c.png",
        "MuxAnalog",
        MuxAnalog::construct );
}

MuxAnalog::MuxAnalog( QObject* parent, QString type, QString id )
         : LogicComponent( parent, type, id )
         , eElement( id )
{
    Q_UNUSED( MuxAnalog_properties );

    setLabelPos(-16,-16, 0);
    
    Pin* pin = new Pin( 180, QPoint( -24, 8 ), m_id+"-PinInput", 0, this );
    pin->setLabelText( " Z" );
    pin->setLabelColor( QColor( 0, 0, 0 ) );
    m_inputPin = pin;
    
    pin = new Pin( 180, QPoint( -24, 16 ), m_id+"-PinEnable", 0, this );
    pin->setLabelText( " En" );
    pin->setLabelColor( QColor( 0, 0, 0 ) );
    pin->setInverted( true );
    m_enablePin = pin;
    
    setAddrBits( 3 );
}
MuxAnalog::~MuxAnalog()
{
    for( eResistor* res : m_resistor ) delete res;
    //for( ePin* pin : m_chanPin ) delete pin;
    //for( ePin* pin : m_addrPin ) delete pin;
}

QList<propGroup_t> MuxAnalog::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Address_Bits", tr("Address Size"),"Bits"} );
    mainGroup.propList.append( {"Impedance", tr("Impedance"),"Ω"} );
    return {mainGroup};
}

void MuxAnalog::stamp()
{
    eNode* enode = m_inputPin->getEnode();
    if( enode ) enode->setSwitched( true );

    for( int i=0; i<m_channels; ++i )
    {
        m_ePin[i]->setEnode( enode );

       eNode* node = m_chanPin[i]->getEnode();
       if( node ) node->setSwitched( true );
    }

    enode = m_enablePin->getEnode();
    if( enode ) enode->voltChangedCallback( this );

    for( int i=0; i<m_addrBits; ++i )
    {
        enode = m_addrPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_enabled = false;
}

void MuxAnalog::voltChanged()
{
    bool enabled = m_enablePin->getVolt() < 2.5;

    m_enabled = enabled;

    int address = 0;
    for( int i=0; i<m_addrBits; ++i )
    {
        bool state = (m_addrPin[i]->getVolt()>2.5);
        if( state ) address += pow( 2, i );
    }
    m_address = address;

    Simulator::self()->addEvent( m_propDelay, this );
}

void MuxAnalog::runEvent()
{
    for( int i=0; i<m_channels; ++i )
    {
        if( m_enabled && (i == m_address) )
        {
            if( m_resistor[i]->admit() == 0 )
                m_resistor[i]->setAdmit( m_admit );
        }
        else if( m_resistor[i]->admit() != 0 )
            m_resistor[i]->setAdmit( 0 );
    }
}

double MuxAnalog::resist()
{
    return 1/m_admit;
}

void MuxAnalog::setResist( double r )
{
    m_admit = 1/r;
}

void MuxAnalog::setAddrBits( int bits )
{
    if( bits == m_addrBits ) return;
    if( bits < 1 ) bits = 1;
    
    int channels = pow( 2, bits );
    
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    
    if( bits < m_addrBits ) deleteAddrBits( m_addrBits-bits );
    else                    createAddrBits( bits-m_addrBits );

    if  ( channels < m_channels ) deleteResistors( m_channels-channels );
    else                          createResistors( channels-m_channels );
    
    m_channels = channels;
    
    int rside = m_channels*8+8;
    int size = 5*8 + bits*8;
    if( rside > size ) size = rside;

    m_area = QRect( -2*8, 0, 4*8, size );
    
    Pin* pin =(static_cast<Pin*>(m_enablePin));
    pin->setPos( QPoint(-3*8,4*8+bits*8 ) );
    pin->isMoved();
    pin->setLabelPos();

    Circuit::self()->update();
}

void MuxAnalog::createAddrBits( int c )
{
    int start = m_addrBits;
    m_addrBits = m_addrBits+c;
    m_addrPin.resize( m_addrBits );

    for( int i=start; i<m_addrBits; i++ )
    {
        QPoint pinpos = QPoint(-3*8,3*8+i*8 );
        Pin* pin = new Pin( 180, pinpos, m_id+"-pinAddr"+QString::number(i), 0, this);
        pin->setLabelText( " A"+QString::number(i) );
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        m_addrPin[i] = pin;
    }
    //update();
}

void MuxAnalog::deleteAddrBits( int d )
{
    int start = m_addrBits-d;

    for( int i=start; i<m_addrBits; i++ )
    {
        Pin* pin = (static_cast<Pin*>(m_addrPin[i]));
        if( pin->connector() ) pin->connector()->remove();
        
        delete pin;
    }
    m_addrBits = m_addrBits-d;
    m_addrPin.resize( m_addrBits );
    //Circuit::self()->update();
}

void MuxAnalog::createResistors( int c )
{
    int start = m_channels;
    m_channels = m_channels+c;
    m_resistor.resize( m_channels );
    m_chanPin.resize( m_channels );
    m_ePin.resize( m_channels );

    for( int i=start; i<m_channels; i++ )
    {
        QString reid = m_id;
        reid.append(QString("-resistor"+QString::number(i)));
        m_resistor[i] = new eResistor( reid );
        
        QString pinId = reid+"-pinL";
        m_ePin[i] = new ePin( pinId, 0 );
        m_resistor[i]->setEpin( 0, m_ePin[i] );
        
        pinId = m_id+"-pinY"+QString::number(i);
        QPoint pinpos = QPoint( 3*8, 8+i*8 );
        Pin* pin = new Pin( 0, pinpos, pinId, 0, this);
        pin->setLabelText( "Y"+QString::number(i)+" " );
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        m_resistor[i]->setEpin( 1, pin );
        m_chanPin[i] = pin;

        m_resistor[i]->setAdmit( 0 );
    }
    //update();
}

void MuxAnalog::deleteResistors( int d )
{
    int start = m_channels-d;

    for( int i=start; i<m_channels; i++ )
    {
        Pin* pin = static_cast<Pin*>(m_chanPin[i]);
        if( pin->connector() ) pin->connector()->remove();
        
        delete pin;
        delete m_ePin[i];
        delete m_resistor[i];
    }
    m_resistor.resize( start );
    m_chanPin.resize( start );
    m_ePin.resize( start );
    //Circuit::self()->update();
}

void MuxAnalog::remove()
{
    if( m_inputPin->isConnected() )  (static_cast<Pin*>(m_inputPin))->connector()->remove();
    if( m_enablePin->isConnected() ) (static_cast<Pin*>(m_enablePin))->connector()->remove();
    
    for( int i=0; i<m_addrBits; i++ ) 
    {
        Pin* pin = static_cast<Pin*>(m_addrPin[i]);
        if( pin->connector() ) pin->connector()->remove();
    }
    deleteResistors( m_channels );
    deleteAddrBits( m_addrBits );
    
    Component::remove();
}

void MuxAnalog::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    
    p->drawRoundRect( m_area, 4, 4 );
}
#include "moc_mux_analog.cpp"

