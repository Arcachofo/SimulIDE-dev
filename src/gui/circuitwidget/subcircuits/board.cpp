/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "board.h"
#include "circuit.h"

BoardSubc::BoardSubc( QObject* parent, QString type, QString id )
         : SubCircuit( parent, type, id )
{
    m_subcType = Chip::Board;
    //m_shield = NULL;
}
BoardSubc::~BoardSubc(){}

void BoardSubc::attachShield( SubCircuit* shield )
{
    if( !m_shields.contains( shield ) ) m_shields.append( shield );
}

void BoardSubc::remove()
{
    for( SubCircuit* shield : m_shields ) // there is a shield attached to this
    {
        shield->setParentItem( NULL );
        /// m_shield->setBoard( NULL );
        Circuit::self()->removeComp( shield );
    }
    SubCircuit::remove();
}

#include "moc_board.cpp"
