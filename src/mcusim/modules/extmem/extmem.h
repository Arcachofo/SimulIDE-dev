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

class IoPin;

class MAINMODULE_EXPORT ExtMemModule : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        ExtMemModule( eMcu *mcu, QString name );
        ~ExtMemModule();

        enum memState_t{
            mem_IDLE=0,
            mem_LAEN,
            mem_ADDR,
            mem_LADI,
            mem_DATA,
            mem_READ
        };
        enum {
            RW=1,
            RE=1<<1,
            EN=1<<2,
            LA=1<<3
        };

        //virtual void initialize() override;
        //virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void reset() override;

        void read( uint32_t addr, uint8_t mode );
        void write( uint32_t addr, uint32_t data );

        uint32_t getData() { return m_data; }

        void setLaEnSetTime(  uint64_t t ) { m_laEnSetTime = t; }
        void setAddrSetTime(  uint64_t t ) { m_addrSetTime = t; }
        void setLaEnEndTime(  uint64_t t ) { m_laEnEndTime = t; }
        void setReadSetTime(  uint64_t t ) { m_readSetTime = t; }
        void setWriteSetTime( uint64_t t ) { m_writeSetTime = t; }
        void setReadBusTime(  uint64_t t ) { m_readBusTime = t; }

    protected:

        bool m_read;

        uint8_t m_dataSize;
        uint8_t m_addrSize;

        uint8_t  m_addrH;
        uint32_t m_addr;
        uint32_t m_data;
                                // All these times Respect to cycle start:
                                // |--CYCLE-----------------------------------|
        uint64_t m_laEnSetTime; // |--laEnSet--|
        uint64_t m_addrSetTime; // |--addrSet------|
        uint64_t m_laEnEndTime; // |--laEnEnd----------|
        uint64_t m_readSetTime; // |--readSet--------------|...|
        uint64_t m_writeSetTime;// |--writeSet-------------|...|
        uint64_t m_readBusTime; // |--readBus-------------------------|
        uint64_t m_dataTime;    // to store previous times

        //uint64_t m_rwPinSetTime;
        //uint64_t m_rwPinDurTime;

        memState_t m_memState;
        uint8_t    m_readMode;

        IoPin* m_rwPin; // !Write / Read Pin
        IoPin* m_rePin; // !Read Pin
        IoPin* m_enPin; // !Enable Pin
        IoPin* m_laPin; //  Latch Enable in Low+High Address mode (8051 ALE)
                        //  Acting on Data Pins. Active High

        std::vector<IoPin*> m_addrPin;
        std::vector<IoPin*> m_dataPin;
};

#endif
