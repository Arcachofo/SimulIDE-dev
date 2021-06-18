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

#include "shiftreg.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* ShiftReg::construct( QObject* parent, QString type, QString id )
{
    return new ShiftReg( parent, type, id );
}

LibraryItem* ShiftReg::libraryItem()
{
    return new LibraryItem(
        tr( "Shift Reg." ),
        tr( "Logic/Arithmetic" ),
        "1to3.png",
        "ShiftReg",
        ShiftReg::construct );
}

ShiftReg::ShiftReg( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 9;

    QStringList pinList;

    pinList // Inputs:
            << "IL03 DI"
            << "IL05>"
            << "IL07 Rst"
            ;
    init( pinList );

    setNumOuts( 8, "Q" );
    createOePin( "IU01OE ", id+"-in4");

    m_clkPin = m_inPin[1];

    setResetInv( true );         // Invert Reset Pin
}
ShiftReg::~ShiftReg(){}

QList<propGroup_t> ShiftReg::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Clock_Inverted", tr("Clock Inverted"),""} );
    mainGroup.propList.append( {"Reset_Inverted", tr("Reset Inverted"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void ShiftReg::stamp()
{
    m_inPin[2]->changeCallBack( this );
    LogicComponent::stamp();
}

void ShiftReg::voltChanged()
{
    updateOutEnabled();
    updateClock();

    bool clkRising = (m_clkState == Clock_Rising);// Get Clk to don't miss any clock changes
    bool     reset = m_inPin[2]->getInpState();

    if( reset ) m_nextOutVal = 0;        // Reset shift register
    else if( clkRising )                 // Clock rising edge
    {
        m_nextOutVal <<= 1;

        bool data = m_inPin[0]->getInpState();
        if( data ) m_nextOutVal += 1;
    }
    sheduleOutPuts( this );
}

void ShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_inPin[2]->setInverted( inv );
}

#include "moc_shiftreg.cpp"
