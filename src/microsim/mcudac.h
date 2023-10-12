/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUDAC_H
#define MCUDAC_H

#include "mcumodule.h"
#include "e-element.h"

class eMcu;
class McuPin;

class McuDac: public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuDac( eMcu* mcu, QString name );
        ~McuDac();

        virtual void initialize() override;

        virtual void outRegChanged( uint8_t ){;}

    protected:

        bool m_enabled;
        bool m_outVoltEn;

        double m_outVolt;
        double m_vRefP;      // Positive Reference Voltage
        double m_vRefN;      // Negative Reference Voltage

        uint8_t* m_dacReg;   // Output value Register

        uint8_t m_outVal;

        std::vector<McuPin*> m_pins;

        McuPin* m_pRefPin;   // Positive Vref Pin
        McuPin* m_nRefPin;   // Negative Vref Pin
        McuPin* m_outPin;    // Output Pin
};

#endif
