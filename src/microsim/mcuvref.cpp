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

#include "mcuvref.h"
#include "e_mcu.h"

McuVref::McuVref( eMcu* mcu, QString name )
       : McuModule( mcu, name )
       , eElement( mcu->getId()+"-"+name )
{
    m_pinOut = NULL;
}
McuVref::~McuVref(){}

void McuVref::initialize()
{
    m_enabled = false;
    m_mode = 0;
    m_vref = 0;
    //setMode( 0 );
}

/*void McuVref::setMode( uint8_t mode )
{
    m_mode = mode;
}*/

void McuVref::callBack( McuModule* mod, bool call ) // Add Modules to be called at Interrupt raise
{
    if( call )
    { if( !m_callBacks.contains( mod ) ) m_callBacks.append( mod ); }
    else m_callBacks.removeAll( mod );
}
