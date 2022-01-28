/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "mcucomparator.h"
#include "e_mcu.h"

McuComp::McuComp( eMcu* mcu, QString name )
       : McuModule( mcu, name )
       , eElement( mcu->getId()+"-"+name )
{
    m_pinP = NULL;
    m_pinN = NULL;
    m_pinOut = NULL;
    m_enabled = false;
}
McuComp::~McuComp(){}

void McuComp::initialize()
{
    m_fixVref = false;
    m_enabled = true;
    m_compOut = false;
    m_vref = 0;
    setMode( 0 );
}

void McuComp::callBackDoub( double vref )
{
    m_vref = vref;
    if( m_enabled ) voltChanged();
}
