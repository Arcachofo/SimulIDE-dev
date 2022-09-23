/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ECLOCKEDDEVICE_H
#define ECLOCKEDDEVICE_H

#include "component.h"
#include "e-element.h"

enum clkState_t{
    Clock_Low = 0,
    Clock_Rising=1,
    Clock_Allow=1,
    Clock_High,
    Clock_Falling,
};

enum trigger_t {
    None = 0,
    Clock,
    InEnable
};

class eElement;
class IoPin;

class MAINMODULE_EXPORT eClockedDevice : public eElement
{
    public:
        eClockedDevice( QString id );
        ~eClockedDevice();

        virtual void stamp() override;

        trigger_t trigger() { return m_trigger; }
        virtual void setTrigger( trigger_t trigger );

        void setClockPin( IoPin* clockPin ) { m_clkPin = clockPin; }

        bool clockInv();
        void setClockInv( bool inv );

        void updateClock();

        void remove();

    protected:
        bool m_clock;

        clkState_t m_clkState;
        trigger_t  m_trigger;

        IoPin* m_clkPin;
};

#endif
