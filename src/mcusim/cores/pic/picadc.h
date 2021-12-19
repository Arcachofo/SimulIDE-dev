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

#ifndef PICADC_H
#define PICADC_H

#include "mcuadc.h"
#include "mcutypes.h"

class eMcu;
class McuPin;
class PicTimer80;
class PicTimer16bit;
//class McuOcUnit;

class MAINMODULE_EXPORT PicAdc : public McuAdc
{
    public:
        PicAdc( eMcu* mcu, QString name );
        ~PicAdc();

 static PicAdc* createAdc( eMcu* mcu, QString name );

        virtual void initialize() override;

        virtual void configureA( uint8_t newADCON0 ) override;
        //virtual void configureB( uint8_t newADCON1 ) override;
        //virtual void callBack() override { if( !m_converting ) startConversion(); }

    protected:
        virtual void endConversion() override;

        uint8_t m_mode;

        regBits_t m_ADON;
        regBits_t m_ADSC;
        regBits_t m_CHS;
        regBits_t m_GODO;

        regBits_t m_ADFM;
};

class MAINMODULE_EXPORT PicAdc0 : public PicAdc
{
    public:
        PicAdc0( eMcu* mcu, QString name );
        ~PicAdc0();

        //virtual void initialize() override;

        //virtual void configureA( uint8_t newADCSRA ) override;
        virtual void configureB( uint8_t newADCSRB ) override;

    protected:
        virtual void updtVref() override;

        regBits_t m_PCFG;
};

class MAINMODULE_EXPORT PicAdc1 : public PicAdc
{
    public:
        PicAdc1( eMcu* mcu, QString name );
        ~PicAdc1();

        virtual void configureB( uint8_t newADCSRB ) override;

        void setANSEL( uint8_t newANSEL );
        void setANSELH( uint8_t newANSELH );
        void updtANSEL();

    protected:
        virtual void updtVref() override;

        uint8_t* m_ANSEL;
        uint8_t* m_ANSELH;

        regBits_t m_VCFG;
};

class MAINMODULE_EXPORT PicAdc2 : public PicAdc0
{
    public:
        PicAdc2( eMcu* mcu, QString name );
        ~PicAdc2();

    protected:
};

class MAINMODULE_EXPORT PicAdc3 : public PicAdc1
{
    public:
        PicAdc3( eMcu* mcu, QString name );
        ~PicAdc3();

    protected:
};
#endif
