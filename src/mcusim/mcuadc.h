/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef MCUADC_H
#define MCUADC_H

#include "mcumodule.h"
#include "e-element.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuAdc : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuAdc( eMcu* mcu, QString name );
        ~McuAdc();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void setChannel( uint8_t val ){;}

        void startConversion();

    protected:
        virtual double getVref()=0;
        virtual void endConversion(){;}

        bool m_enabled;
        bool m_converting;
        bool m_leftAdjust;

        uint16_t m_adcValue; // Value obtained in last conversion
        uint16_t m_maxValue; // Maximum value = 2^m_bits

        uint8_t* m_ADCL; // Actual ram for ADC Reg. Low byte
        uint8_t* m_ADCH; // Actual ram for ADC Reg. High byte

        std::vector<McuPin*> m_adcPin; // ADC Pins

        std::vector<uint16_t> m_prescList; // Prescaler values

        uint64_t m_convTime;          // Time to complete a conversion in ps

        //int m_bits;                   // ADC resolution in bits
        uint m_channel;                // Channel number for current conversion

        //double m_vRef;                // Reference Voltage
};

#endif
