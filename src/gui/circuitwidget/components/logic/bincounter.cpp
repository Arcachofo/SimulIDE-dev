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
#include "iopin.h"

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
    
    m_clkPin = m_inPin[0];     // eClockedDevice
    m_resetPin = m_inPin[1];
    m_setPin   = m_inPin[2];

    setSrInv( true );            // Invert Reset Pin
    useSetPin( false );          // Don't use Set Pin
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
    m_resetPin->changeCallBack( this );
    m_setPin->changeCallBack( this );

    LogicComponent::stamp();
}

void BinCounter::initialize()
{
    m_Counter = 0;
    LogicComponent::initialize();
}

void BinCounter::voltChanged()
{
    bool clkRising = (LogicComponent::getClockState() == Clock_Rising);

    if( m_resetPin->getInpState() ) // Reset
    {
       m_Counter = 0;
       m_nextOutVal = 0;
    }
    else if( m_setPin->getInpState() ) // Set
    {
       m_Counter = m_TopValue;
       m_nextOutVal = 1;
    }
    else if( clkRising )
    {
        m_Counter++;

        if(      m_Counter == m_TopValue ) m_nextOutVal = 1;
        else if( m_Counter > m_TopValue )
        {
            m_Counter = 0;
            m_nextOutVal = 0;
        }
    }
    IoComponent::sheduleOutPuts( this );
}

void BinCounter::setSrInv( bool inv )
{
    m_resetInv = inv;
    m_resetPin->setInverted( inv );

    if( m_pinSet ) m_setPin->setInverted( inv );
    else           m_setPin->setInverted( false );
}

void BinCounter::useSetPin( bool set )
{
    m_pinSet = set;
    if( !set ) m_setPin->removeConnector();

    m_setPin->setVisible( set );
    setSrInv( m_resetInv );
}

#include "moc_bincounter.cpp"
