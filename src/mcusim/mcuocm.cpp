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

#include "mcuocm.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "simulator.h"
#include "datautils.h"
#include "regwatcher.h"

McuOcm::McuOcm( eMcu* mcu, QString name )
      : McuPrescaled( mcu, name )
      , eElement( mcu->getId()+"-"+name )
{
    m_OC1 = NULL;
    m_OC2 = NULL;
    m_oPin = NULL;
}
McuOcm::~McuOcm(){}

void McuOcm::initialize()
{
    m_oc1Active = false;
    m_oc2Active = false;
    m_state1 = false;
    m_state2 = false;

    if( m_OC1 ) m_oPin = m_OC1->getPin();
}

void McuOcm::setOcActive( McuOcUnit* oc, bool a ) // OC units call when activated/deactivated
{
    if( oc == m_OC1 ) m_oc1Active = a;
    if( oc == m_OC2 ) m_oc2Active = a;

    bool ctrl = !(m_oc1Active && m_oc2Active); // If both Oc units active, then OCM takes control
    m_OC1->setCtrlPin( ctrl );
    m_OC2->setCtrlPin( ctrl );
}

void McuOcm::setState( McuOcUnit* oc, bool s ) // OC units call at state changes
{
    if( oc == m_OC1 ) m_state1 = s;
    if( oc == m_OC2 ) m_state2 = s;
    OutputOcm();
}

