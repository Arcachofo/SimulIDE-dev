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

#include "fulladder.h"
#include "itemlibrary.h"

Component* FullAdder::construct(QObject *parent, QString type, QString id)
{
    return new FullAdder(parent, type, id);
}

LibraryItem* FullAdder::libraryItem()
{
    return new LibraryItem(
        tr( "Full Adder" ),
        tr ("Logic/Arithmetic"),
        "2to2.png",
        "FullAdder",
        FullAdder::construct );
}

FullAdder::FullAdder(QObject *parent, QString type, QString id) 
          : LogicComponent( parent, type, id )
{
    m_width  = 3;
    m_height = 4;

    QStringList pinList;
    pinList
        << "IL01 A"
        << "IL03 B"
        << "IR01Ci "

        // Outputs:
        << "OR02S "
        << "OR03Co "
        ;
    init( pinList );
}
FullAdder::~FullAdder(){}

void FullAdder::stamp()
{
    for( uint i=0; i<m_inPin.size(); ++i )m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp();
}

void FullAdder::voltChanged()
{
    bool X  = m_inPin[0]->getInpState();
    bool Y  = m_inPin[1]->getInpState();
    bool Ci = m_inPin[2]->getInpState();

    bool sum = (X ^ Y) ^ Ci;                    // Sum
    bool co  = (X & Ci) | (Y & Ci) | (X & Y);   // Carry out

    m_nextOutVal = 0;
    if( sum ) m_nextOutVal += 1;
    if( co  ) m_nextOutVal += 2;
    sheduleOutPuts( this );
}

#include "moc_fulladder.cpp"
