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

#ifndef AVRADC_H
#define AVRADC_H

#include "mcuadc.h"
#include "mcutypes.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT AvrAdc : public McuAdc
{
    public:
        AvrAdc( eMcu* mcu, QString name );
        ~AvrAdc();

        virtual void initialize() override;

        virtual void configureA( uint8_t val ) override;
        virtual void configureB( uint8_t val ) override;
        virtual void setChannel( uint8_t val ) override;

    protected:
        virtual double getVref() override;
        virtual void endConversion() override;

        bool m_leftAdjust;
        bool m_autoTrigger;

        uint8_t m_refSelect;

        uint8_t* m_ADCSRA;
        regBits_t m_ADSC;
        regBits_t m_ADIF;

        McuPin* m_aRefPin;
        McuPin* m_aVccPin;
};

#endif
