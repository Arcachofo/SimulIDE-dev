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
    wgmNORMAL = 0,
    wgmPHASE,
    wgmCTC,
    wgmFAST,
};

class MAINMODULE_EXPORT AvrTimer
{
    public:
        AvrTimer();
        ~AvrTimer();

 static McuTimer* makeTimer( eMcu* mcu, QString name );
};

class AvrOcUnit;

class MAINMODULE_EXPORT AvrTimer0 : public McuTimer
{
    public:
        AvrTimer0( eMcu* mcu, QString name );
        ~AvrTimer0();

        virtual void runEvent() override;

        virtual void initialize() override;
        virtual void configureA( uint8_t val ) override;
        virtual void configureB( uint8_t val ) override;

        void OCRAchanged( uint8_t val );

        virtual void addocUnit( McuOcUnit* ocUnit ) override;

    protected:
        void configureClock();
        void updtWgm();

        wgmMode_t m_wgmMode;
        bool      m_WGM02;

        uint8_t* m_ocr0a;

        McuOcUnit* m_OCA;
        McuOcUnit* m_OCB;
};

#endif
