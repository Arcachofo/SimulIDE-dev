/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "bincounter.h"
#include "itemlibrary.h"
#include "connector.h"
#include "e-source.h"
#include "pin.h"

static const char* BinCounter_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Max Value")
};

Component *BinCounter::construct(QObject *parent, QString type, QString id)
{
    return new BinCounter(parent, type, id);
}

LibraryItem* BinCounter::libraryItem()
{
    return new LibraryItem(
        tr( "Counter" ),
        tr ("Logic/Arithmetic"),
        "2to1.png",
        "Counter",
        BinCounter::construct );
}

BinCounter::BinCounter(QObject *parent, QString type, QString id) 
          : LogicComponent( parent, type, id )
          , eLogicDevice( id )
{
    Q_UNUSED( BinCounter_properties );
    
    m_TopValue = 1;

    m_width  = 3;
    m_height = 3;

    QStringList pinList;
    pinList
      << "IL01>"
      << "IL02 R"
      << "IU01S"
      << "OR01Q"
    ;
    init( pinList );
    
    eLogicDevice::createClockPin( m_inPin[0] );      // Input Clock
    eLogicDevice::createInput( m_inPin[1] );         // Input Reset
    eLogicDevice::createInput( m_inPin[2] );         // Input Set
    eLogicDevice::createOutput( m_outPin[0] );       // Output Q

    setSrInv( true );                             // Invert Reset Pin
    setPinSet( false );                          // Don't use Set Pin
}
BinCounter::~BinCounter(){}

QList<propGroup_t> BinCounter::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Pin_SET", tr("Use Set Pin"),""} );
    mainGroup.propList.append( {"Clock_Inverted", tr("Clock Inverted"),""} );
    mainGroup.propList.append( {"Reset_Inverted", tr("Set / Reset Inverted"),""} );
    mainGroup.propList.append( {"Max_Value", tr("Count to"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void BinCounter::stamp()
{
    eNode* enode = m_input[0]->getEpin(0)->getEnode();      // Reset pin
    if( enode ) enode->voltChangedCallback( this );

    enode = m_input[1]->getEpin(0)->getEnode();              // Set pin
    if( enode ) enode->voltChangedCallback(this);

    eLogicDevice::stamp();
}

void BinCounter::initialize()
{
    m_Counter = 0;
    eLogicDevice::initialize();
}

void BinCounter::voltChanged()
{
    bool clkRising = (eLogicDevice::getClockState() == Clock_Rising);

    if( eLogicDevice::getInputState( 0 ) == true ) // Reset
    {
       m_Counter = 0;
       m_nextOutVal = 0;
    }
    else if( eLogicDevice::getInputState( 1 ) == true ) // Set
    {
       m_Counter = m_TopValue;
       m_nextOutVal = 1;
    }
    else if( clkRising )
    {
        m_Counter++;

        if( m_Counter == m_TopValue )
        {
            m_nextOutVal = 1;
        }
        else if( m_Counter > m_TopValue )
        {
            m_Counter = 0;
            m_nextOutVal = 0;
        }
    }
    sheduleOutPuts();
}

void BinCounter::setSrInv( bool inv )
{
    m_resetInv = inv;
    m_input[0]->setInverted( inv );       // Input Reset
    if( m_pinSet ) m_input[1]->setInverted( inv );       // Input Set
    else           m_input[1]->setInverted( false );
}

void BinCounter::setPinSet( bool set )
{
    m_pinSet = set;
    if( !set && m_inPin[2]->connector() ) m_inPin[2]->connector()->remove();

    m_inPin[2]->setVisible( set );
    setSrInv( m_resetInv );
}

#include "moc_bincounter.cpp"
