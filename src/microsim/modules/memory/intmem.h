/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INTMemModule_H
#define INTMemModule_H

#include "mcumodule.h"
#include "e-element.h"

class IoPin;

class MAINMODULE_EXPORT IntMemModule : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        IntMemModule( eMcu *mcu, QString name );
        ~IntMemModule();

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
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        void step();
        virtual void reset() override;

    protected:
        void write( bool w );
        void enableOutputs( bool en );
        void runOutputs();
        void sheduleOutPuts();

        bool m_we;
        bool m_cs;
        bool m_write;
        bool m_asynchro;

        uint8_t  m_addrH;
        uint32_t m_addr;
        uint32_t m_data;

        uint m_outValue;
        uint m_nextOutVal;
        uint64_t m_propDelay; // Propagation delay

        memState_t m_memState;

        IoPin* m_rwPin;  // !Write / Read Pin
      //IoPin* m_rePin;  // !Read Pin
        IoPin* m_cshPin; // Enable Pin
        IoPin* m_cslPin; // !Enable Pin
        IoPin* m_clkPin; // Clock Pin

        std::vector<IoPin*> m_addrPin;
        std::vector<IoPin*> m_dataPin;
};

#endif
