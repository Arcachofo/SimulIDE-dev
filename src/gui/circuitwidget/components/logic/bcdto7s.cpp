/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "bcdto7s.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

const uint8_t BcdTo7S::m_values[]={
        0b00111111,
        0b00000110,
        0b01011011,
        0b01001111,
        0b01100110,
        0b01101101,
        0b01111101,
        0b00000111,
        0b01111111,
        0b01101111,
        0b01110111,
        0b01111100,
        0b00111001,
        0b01011110,
        0b01111001,
        0b01110001,
        0b00000000
};

Component* BcdTo7S::construct( QObject* parent, QString type, QString id )
{
        return new BcdTo7S( parent, type, id );
}

LibraryItem* BcdTo7S::libraryItem()
{
    return new LibraryItem(
        tr( "Bcd To 7S." ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "BcdTo7S",
        BcdTo7S::construct );
}

BcdTo7S::BcdTo7S( QObject* parent, QString type, QString id )
       : LogicComponent( parent, type, id )
       , eElement( id )
{
    m_width  = 4;
    m_height = 8;

    QStringList pinList;

    pinList // Inputs:
            
            << "IL03 S0"
            << "IL04 S1"
            << "IL05 S2"
            << "IL06 S3"

            << "IU01OE "
            
            // Outputs:
            << "OR01a  "
            << "OR02b  "
            << "OR03c  "
            << "OR04d "
            << "OR05e  "
            << "OR06f  "
            << "OR07g "
            ;
    init( pinList );

    setOePin(  m_inPin[4] );    // IOutput Enable
}
BcdTo7S::~BcdTo7S(){}

QList<propGroup_t> BcdTo7S::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Inverted", tr("Invert Outputs"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void BcdTo7S::stamp()
{
    for( int i=0; i<4; ++i )
    {
        eNode* enode = m_inPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_nextOutVal = m_values[0];
    m_changed = true;

    uint8_t value = m_values[0];
    for( int i=0; i<7; ++i ) m_outPin[i]->setOutState( value & (1<<i) );
}

void BcdTo7S::voltChanged()
{
    LogicComponent::updateOutEnabled();

    m_changed = true;

    bool a = m_inPin[0]->getInpState();
    bool b = m_inPin[1]->getInpState();
    bool c = m_inPin[2]->getInpState();
    bool d = m_inPin[3]->getInpState();

    int digit = a*1+b*2+c*4+d*8;
    m_nextOutVal = m_values[digit];

    if( m_outPin.size() ) sheduleOutPuts( this );
}

void BcdTo7S::runEvent()
{
    IoComponent::runOutputs();
}

#include "moc_bcdto7s.cpp"
