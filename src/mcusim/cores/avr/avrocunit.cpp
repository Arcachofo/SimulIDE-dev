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

#include "avrocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "mcuocm.h"

AvrOcUnit::AvrOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
}
AvrOcUnit::~AvrOcUnit( ){}

void AvrOcUnit::configure( uint8_t val ) // COMNX0,COMNX1
{
    m_mode = getRegBitsVal( val, m_configBitsA );

    bool enabled = m_mode > 0;
    if( m_enabled == enabled ) return;
    m_enabled = enabled;

    if( enabled ){                       // OC Pin connected
        m_ocPin->controlPin( true, false );
        m_ocPin->setOutState( false );
   }else{                               // OC Pin disconnected
        m_ocPin->controlPin( false, false );
   }
    if( m_ocm ) m_ocm->setOcActive( this, enabled );
    else        m_ctrlPin = enabled;
}

void AvrOcUnit::setPinSate( bool state )
{
    if( m_ctrlPin )  m_ocPin->setOutState( state );
    else if( m_ocm ) m_ocm->setState( this, state );
}
