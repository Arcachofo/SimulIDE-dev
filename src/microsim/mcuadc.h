/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUADC_H
#define MCUADC_H

#include "mcuprescaled.h"
#include "e-element.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuAdc : public McuPrescaled, public eElement
{
        friend class McuCreator;

    public:
        McuAdc( eMcu* mcu, QString name );
        ~McuAdc();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void setChannel( uint8_t val ){;}

        virtual void startConversion();

    protected:
        virtual void updtVref();
        virtual void specialConv();
        virtual void endConversion(){;}

        bool m_enabled;
        bool m_converting;
        bool m_leftAdjust;

        uint16_t m_adcValue; // Value obtained in last conversion
        uint16_t m_maxValue; // Maximum value = 2^m_bits

        uint8_t* m_ADCL; // Actual ram for ADC Reg. Low byte
        uint8_t* m_ADCH; // Actual ram for ADC Reg. High byte

        std::vector<McuPin*> m_adcPin; // ADC Pins
        std::vector<McuPin*> m_refPin; // Vref Pins
        McuPin* m_pRefPin;             // Positive Vref Pin
        McuPin* m_nRefPin;             // Negative Vref Pin

        uint64_t m_convTime;           // Time to complete a conversion in ps

        //int m_bits;                  // ADC resolution in bits
        uint m_channel;                // Channel number for current conversion
        uint m_chOffset;

        double m_fixedVref;
        double m_vRefP;                // Positive Reference Voltage
        double m_vRefN;                // Positive Reference Voltage
};

#endif
