/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef AVRTIMER_H
#define AVRTIMER_H

#include "mcutimer.h"

enum wgmMode_t{
    wgmNORM = 0,
    wgmPHAS,
    wgmCTC,
    wgmFAST,
};

class MAINMODULE_EXPORT AvrTimer : public McuTimer
{
    public:
        AvrTimer( eMcu* mcu, QString name );
        ~AvrTimer();

 static McuTimer* createTimer( eMcu* mcu, QString name, int type );

        virtual void initialize() override;

        virtual void addOcUnit( McuOcUnit* ocUnit ) override;
        virtual McuOcUnit* getOcUnit( QString name ) override;

        virtual void configureA( uint8_t newTCCRXA ) override;
        virtual void configureB( uint8_t newTCCRXB ) override;

    protected:
        virtual void updtWgm()=0;
        virtual void setOCRXA( QString reg )=0;
        virtual void configureClock();
        void configureExtClock();
        void configureOcUnits( bool wgm3 );

        wgmMode_t m_wgmMode;
        uint8_t m_WGM10;
        uint8_t m_WGM32;

        McuOcUnit* m_OCA;
        McuOcUnit* m_OCB;
        McuOcUnit* m_OCC;

        uint8_t* m_ocrxaL;
        uint8_t* m_ocrxaH;
};

//class AvrOcUnit;

class MAINMODULE_EXPORT AvrTimer8bit : public AvrTimer
{
    public:
        AvrTimer8bit( eMcu* mcu, QString name );
        ~AvrTimer8bit();

        virtual void OCRXAchanged( uint8_t val );

    protected:
        virtual void updtWgm() override;
        virtual void setOCRXA( QString reg ) override;
};

class MAINMODULE_EXPORT AvrTimer800 : public AvrTimer8bit
{
    public:
        AvrTimer800( eMcu* mcu, QString name );
        ~AvrTimer800();

    protected:
        virtual void configureClock() override;
};

class MAINMODULE_EXPORT AvrTimer801 : public McuTimer
{
    public:
        AvrTimer801( eMcu* mcu, QString name );
        ~AvrTimer801();

        virtual void initialize() override;

        virtual void configureA( uint8_t newTCCR0 ) override;

    protected:
        void configureClock();
};

class MAINMODULE_EXPORT AvrTimer810 : public AvrTimer8bit
{
    public:
        AvrTimer810( eMcu* mcu, QString name );
        ~AvrTimer810();

        virtual void configureA( uint8_t newGTCCR ) override;
        virtual void configureB( uint8_t newTCCR1 ) override;

    protected:
        virtual void configureClock() override;
};


class MAINMODULE_EXPORT AvrTimer820 : public AvrTimer8bit
{
    public:
        AvrTimer820( eMcu* mcu, QString name );
        ~AvrTimer820();

};

class MAINMODULE_EXPORT AvrTimer821 : public AvrTimer8bit
{
    public:
        AvrTimer821( eMcu* mcu, QString name );
        ~AvrTimer821();

        virtual void configureA( uint8_t newTCCR2 ) override;

    protected:
};


class MAINMODULE_EXPORT AvrTimer16bit : public AvrTimer
{
    public:
        AvrTimer16bit( eMcu* mcu, QString name );
        ~AvrTimer16bit();

        virtual void OCRXAchanged( uint8_t val ) { updtWgm(); }

    protected:
        virtual void updtWgm() override;
        virtual void setOCRXA( QString reg ) override;
        virtual void configureClock() override;
        void setICRX( QString reg );

        uint8_t* m_icrxL;
        uint8_t* m_icrxH;
};

#endif
