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

#include "pic14einterrupt.h"
#include "e_mcu.h"
#include "datautils.h"

Pic14eInterrupt::Pic14eInterrupt( QString name, uint16_t vector, eMcu* mcu )
               : PicInterrupt( name, vector, mcu )
{
    m_wReg   = mcu->getReg("WREG");
    m_status = mcu->getReg("STATUS");
    m_bsr    = mcu->getReg("BSR");
    m_pclath = mcu->getReg("PCLATH");
}
Pic14eInterrupt::~Pic14eInterrupt(){}

void Pic14eInterrupt::execute() // Save context
{
    m_wRegSaved   = *m_wReg;
    m_statusSaved = *m_status;
    m_bsrSaved    = *m_bsr;
    m_pclathSaved = *m_pclath;

    Interrupt::execute();
}

void Pic14eInterrupt::exitInt() // Restore context
{
    *m_wReg   = m_wRegSaved;
    *m_status = m_statusSaved;
    *m_bsr    = m_bsrSaved;
    *m_pclath = m_pclathSaved;

    Interrupt::exitInt();
}

