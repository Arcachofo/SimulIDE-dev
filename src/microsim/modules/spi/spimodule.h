/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SPIMODULE_H
#define SPIMODULE_H

#include "e-clocked_device.h"

enum spiMode_t{
    SPI_OFF=0,
    SPI_MASTER,
    SPI_SLAVE
};

class IoPin;

class MAINMODULE_EXPORT SpiModule : public eClockedDevice
{
    public:
        SpiModule( QString name );
        ~SpiModule();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void setMode( spiMode_t mode );

        virtual void endTransaction();
        virtual void StartTransaction();

        virtual void setMosiPin( IoPin* pin ) { m_MOSI = pin; }
        virtual void setMisoPin( IoPin* pin ) { m_MISO = pin; }
        virtual void setSckPin( IoPin* pin )  { m_clkPin = pin; }
        virtual void setSsPin( IoPin* pin )   { m_SS = pin; }

        void setUseSS( bool u ) { m_useSS = u; }

    protected:
        void step();
        void resetSR();
        inline void keepClocking();

        uint64_t m_clockPeriod;   // SPI Clock half period in ps

        bool m_lsbFirst;
        bool m_toggleSck;
        bool m_enabled;
        bool m_useSS;

        clkState_t m_sampleEdge;
        clkState_t m_leadEdge;
        clkState_t m_tailEdge;

        uint8_t m_outBit;
        uint8_t m_inBit;
        uint8_t m_bitCount;

        uint8_t m_srReg;    // Shift Register
        //uint8_t m_rxReg;    // Byte Received

        spiMode_t m_mode;

        IoPin* m_MOSI;
        IoPin* m_MISO;
        //IoPin* m_SCK; // m_clkPin in eClockedDevice
        IoPin* m_SS;

        IoPin* m_dataOutPin;
        IoPin* m_dataInPin;
};
#endif

