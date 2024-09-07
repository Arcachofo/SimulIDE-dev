/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INTOSC_H
#define INTOSC_H

#include "e-element.h"
#include "mcumodule.h"

class eMcu;
class McuPin;

class McuIntOsc : public McuModule, public eElement
{
        //friend class McuCreator;
    public:
        McuIntOsc( eMcu* mcu, QString name );
        ~McuIntOsc();

        virtual void stamp() override;
        virtual void runEvent() override;

        bool extClock() { return m_extClock; }
        void enableExtOsc( bool en );

        bool clockOut() { return m_clkOut; }
        void setClockOut( bool clkOut );

        McuPin* clkOutPin() { return m_clkOutPin; }
        McuPin* clkInPin()  { return m_clkInPin; }
        bool hasClockPins() { return m_clkPin[0] != nullptr; } // Has Multiplexed Clock Pins

        virtual void setPin( int n, McuPin* p );

        void configPins( bool inIo, bool outIo, bool clkOut ); // From Config Word

        virtual void setMultiplier( uint8_t m ) { m_multiplier = m; }

        virtual bool freqChanged(){ return false; }

    protected:
        uint64_t m_psInst;

        uint8_t m_multiplier;

        double m_intOscFreq;
        double m_configFreq;

        bool m_extClock;
        bool m_clkInIO;
        bool m_clkOutIO;
        bool m_clkOut;

        McuPin* m_clkOutPin;
        McuPin* m_clkInPin;

        McuPin* m_clkPin[2];
};
#endif
