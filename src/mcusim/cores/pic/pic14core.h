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

#ifndef PIC14CORE_H
#define PIC14CORE_H

#include "picmrcore.h"

class MAINMODULE_EXPORT Pic14Core : public PicMrCore
{
    public:
        Pic14Core( eMcu* mcu );
        ~Pic14Core();

protected:
    virtual void setBank( uint8_t bank ) override { PicMrCore::setBank( bank ); }

        uint8_t* m_FSR;
        uint8_t m_WregHidden;

        virtual uint8_t GET_RAM( uint16_t addr ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            if( addr == 0 ) addr = *m_FSR;// INDF
            return McuCore::GET_RAM( addr );
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            if( addr == m_PCLaddr ) setPC( v + (m_dataMem[m_PCHaddr]<<8) ); // Writting to PCL
            else if( addr == 0 ) addr = *m_FSR;      // INDF

            McuCore::SET_RAM( addr, v );
        }
};

#endif
