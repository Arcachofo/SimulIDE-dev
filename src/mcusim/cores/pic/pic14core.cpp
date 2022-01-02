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

#include "pic14core.h"
#include "datautils.h"
#include "regwatcher.h"

Pic14Core::Pic14Core( eMcu* mcu )
         : PicMrCore( mcu )
{
    m_stackSize = 8;

    m_FSR = m_mcu->getReg( "FSR" );
    m_OPTION = m_mcu->getReg( "OPTION" );

    m_bankBits = getRegBits( "R0,R1", mcu );
    watchBitNames( "R0,R1", R_WRITE, this, &Pic14Core::setBank, mcu );
}
Pic14Core::~Pic14Core() {}

