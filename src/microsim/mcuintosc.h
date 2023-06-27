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

        virtual void setPin( int n, McuPin* p );
        //McuPin* getClkPin( int n );
        void configPins( bool inIo, bool outIo, bool clkOut ); // From Config Word

    protected:
        uint64_t m_psCycle;

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
