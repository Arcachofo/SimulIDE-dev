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

#ifndef MCUCORE_H
#define MCUCORE_H

#include "e_mcu.h"

#define clear_S_Bit(bit) \
        *m_STATUS &= ~(1<<bit);

#define set_S_Bit( bit ) \
        *m_STATUS |= 1<<bit;

#define write_S_Bit(bit,val) \
        if( val ) *m_STATUS |= 1<<bit; \
        else      *m_STATUS &= ~(1<<bit);

#define STATUS(bit) (*m_STATUS & (1<<bit))

class MAINMODULE_EXPORT McuCore
{
        friend class McuCreator;

    public:
        McuCore( eMcu* mcu );
        virtual ~McuCore();

        virtual void reset()=0;
        virtual void runDecoder()=0;

        virtual void CALL_ADDR( uint32_t addr )=0;
        virtual void RETI() { m_mcu->m_interrupts.retI(); }
        
        uint32_t PC;

    protected:

        eMcu* m_mcu;

        uint8_t*  m_dataMem;
        uint32_t  m_dataMemEnd;
        uint16_t* m_progMem;
        uint32_t  m_progSize;
        uint8_t   m_progAddrSize;

        uint16_t  m_lowDataMemEnd;
        uint16_t  m_regEnd;

        /// uint8_t* m_sreg;  // Mirror of the STATUS register
        uint8_t* m_STATUS;  // MSTATUS register
        uint8_t* m_spl;   // STACK POINTER low byte
        uint8_t* m_sph;   // STACK POINTER high byte
        bool     m_spPre; // STACK pre-increment?
        int      m_spInc; // STACK grows up or down? (+1 or -1)

        /*std::vector<std::function<void()>> m_instructions;

        template <typename T>
        void insertIntr( uint16_t opcode, T* core, void (T::*func)())
        {
            m_instructions.at( opcode ) = std::bind( func, core );
        }*/
        //virtual void createInstructions()=0;
};

#endif
