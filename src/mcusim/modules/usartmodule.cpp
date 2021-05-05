/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "usartmodule.h"
#include "simulator.h"

QHash<QString, UsartM*> UsartM::m_usarts;

UsartM::UsartM( eMcu* mcu, QString name )
      : m_sender( this, name+"Tx" )
      , m_receiver( this, name+"Rx" )
{
    m_mcu = mcu;
    m_name = name;
    m_mode = 0xFF; // Force first mode change.
}
UsartM::~UsartM( ){}

void UsartM::parityError()
{

}

void UsartM::setPeriod( uint64_t period )
{
    m_sender.setPeriod( period );
    m_receiver.setPeriod( period );
}

