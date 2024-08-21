/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRUSI_H
#define AVRUSI_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;
class AvrTimer800;
class McuOcUnit;

class AvrUsi : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        AvrUsi( eMcu* mcu, QString name );
        ~AvrUsi();

        virtual void reset() override;
        virtual void voltChanged() override;

        virtual void configureA( uint8_t ) override;
        virtual void configureB( uint8_t ) override;
        virtual void callBack() override; // Called at Timer0 Compare Match

        void setPins( QString pinStr );

    private:
        inline void stepCounter();
        inline void shiftData();
        inline void setOutput();
        inline void toggleClock();
        void dataRegWritten( uint8_t newUSIDR );

        bool m_twi;
        bool m_spi;
        bool m_timer;
        bool m_extClk;
        bool m_usiClk;
        bool m_clkEdge;
        bool m_clkState;
        bool m_sdaState;
        bool m_DoState;

        uint8_t* m_dataReg;
        uint8_t* m_bufferReg;

        uint8_t m_mode;
        uint8_t m_clockMode;
        uint8_t m_counter;

        // USICR
        regBits_t m_USITC;
        regBits_t m_USICLK;
        regBits_t m_USICS;
        regBits_t m_USIWM;

        // USISR
        regBits_t m_USICNT;
        regBits_t m_USIPF;

        regBits_t m_DIbit;
        regBits_t m_CKbit;

        McuPin* m_DOpin;
        McuPin* m_DIpin;
        McuPin* m_CKpin;

        McuOcUnit* m_t0OCA;

        Interrupt* m_startInte;
};
#endif
