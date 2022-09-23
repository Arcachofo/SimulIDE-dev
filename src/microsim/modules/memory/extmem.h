/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EXTMEMMODULE_H
#define EXTMEMMODULE_H

#include "mcumodule.h"
#include "e-element.h"
#include "mcupin.h"

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

        void read( uint32_t addr, uint8_t mode=RW );
        void write( uint32_t addr, uint32_t data );

        void setAddrBus( uint32_t addr );
        uint32_t getAddress() { return m_addr; }

        void setDataDir( pinMode_t dir );
        void setDataBus( uint32_t data );
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

        McuPin* m_rwPin; // !Write / Read Pin
        McuPin* m_rePin; // !Read Pin
        McuPin* m_enPin; // !Enable Pin
        McuPin* m_laPin; //  Latch Enable in Low+High Address mode (8051 ALE)
                        //  Acting on Data Pins. Active High

        std::vector<McuPin*> m_addrPin;
        std::vector<McuPin*> m_dataPin;
};

#endif
