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
class AvrTimer80;
class AvrTimer16bit;
class McuOcUnit;



class MAINMODULE_EXPORT AvrAdc : public McuAdc
{
    public:
        AvrAdc( eMcu* mcu, QString name );
        ~AvrAdc();

 static AvrAdc* createAdc( eMcu* mcu, QString name );

        virtual void initialize() override;

        virtual void configureA( uint8_t newADCSRA ) override;
        virtual void configureB( uint8_t newADCSRB ) override;
        virtual void setChannel( uint8_t newADMUX ) override;
        virtual void callBack() override { if( !m_converting ) startConversion(); }

    protected:
        virtual void autotriggerConf(){;}
        virtual void endConversion() override;

        bool m_autoTrigger;
        bool m_freeRunning;

        uint8_t m_refSelect;
        uint8_t m_trigger;

        //uint8_t* m_ADCSRA;
        regBits_t m_ADEN;
        regBits_t m_ADSC;
        regBits_t m_ADATE;
        regBits_t m_ADIF;
        regBits_t m_ADPS;

        // ADCSB
        regBits_t m_ADTS;

        //ADMUX
        regBits_t m_ADLAR;
        regBits_t m_REFS;

        McuPin* m_aRefPin;
        McuPin* m_aVccPin;

        AvrTimer80*      m_timer0;
        AvrTimer16bit*  m_timer1;

        McuOcUnit* m_t0OCA;
        McuOcUnit* m_txOCB;
};

class MAINMODULE_EXPORT AvrAdc0 : public AvrAdc
{
    public:
        AvrAdc0( eMcu* mcu, QString name );
        ~AvrAdc0();

        //virtual void initialize() override;

        //virtual void configureA( uint8_t newADCSRA ) override;
        //virtual void configureB( uint8_t newADCSRB ) override;

    protected:
        virtual void autotriggerConf() override;
        virtual double getVref() override;
};

class MAINMODULE_EXPORT AvrAdc1 : public AvrAdc
{
    public:
        AvrAdc1( eMcu* mcu, QString name );
        ~AvrAdc1();

    protected:
        virtual void autotriggerConf() override;
        virtual double getVref() override;
};

class MAINMODULE_EXPORT AvrAdc2 : public AvrAdc0
{
    public:
        AvrAdc2( eMcu* mcu, QString name );
        ~AvrAdc2();

    protected:
        virtual double getVref() override;
};

class MAINMODULE_EXPORT AvrAdc3 : public AvrAdc1
{
    public:
        AvrAdc3( eMcu* mcu, QString name );
        ~AvrAdc3();

    protected:
        virtual double getVref() override;
};
#endif
