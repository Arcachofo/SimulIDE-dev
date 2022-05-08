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

#ifndef EXTMEMMODULE_H
#define EXTMEMMODULE_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;

class MAINMODULE_EXPORT ExtMemModule : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        ExtMemModule( eMcu *mcu, QString name );
        ~ExtMemModule();

        enum memState_t{
            mem_IDLE=0,
            mem_START,
            mem_DATA,
            mem_READ
        };

        //virtual void initialize() override;
        //virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void reset() override;

        void read( uint32_t addr );
        void write( uint32_t addr, uint32_t data );

        uint32_t getData() { return m_data; }

    protected:

        bool m_read;

        uint8_t m_dataSize;
        uint8_t m_addrSize;

        uint32_t m_addr;
        uint32_t m_data;

        // All these times Respect to cycle start:
                                // |------------------------------------------|
        uint64_t m_addrSetTime; // |--addrSet--|
        uint64_t m_readSetTime; // |--readSet--------------------|
        uint64_t m_writeSetTime;// |--writeSet---------------|
        uint64_t m_readBusTime; // |--readBus-------------------------|
        uint64_t m_dataTime;

        //uint64_t m_rwPinSetTime;
        //uint64_t m_rwPinDurTime;

        memState_t m_memState;

        McuPin* m_rwPin;

        std::vector<McuPin*> m_addrPin;
        std::vector<McuPin*> m_dataPin;
};

#endif
