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

#ifndef PICTIMER_H
#define PICTIMER_H

#include "mcutimer.h"

class MAINMODULE_EXPORT PicTimer : public McuTimer
{
    public:
        PicTimer( eMcu* mcu, QString name );
        ~PicTimer();

 static McuTimer* createTimer( eMcu* mcu, QString name );

        virtual void initialize() override;

        virtual void configureA( uint8_t val ) override;
        virtual void configureB( uint8_t val ) override;

    protected:
        virtual void configureClock();
        void configureExtClock();
};


class MAINMODULE_EXPORT PicTimer8bit : public PicTimer
{
    public:
        PicTimer8bit( eMcu* mcu, QString name );
        ~PicTimer8bit();

    protected:
};

class MAINMODULE_EXPORT PicTimer0 : public PicTimer8bit
{
    public:
        PicTimer0( eMcu* mcu, QString name );
        ~PicTimer0();

        virtual void initialize() override;
        virtual void configureA( uint8_t NewOPTION ) override;

    protected:

        regBits_t m_T0CS;
        regBits_t m_T0SE;
        regBits_t m_PSA;
        regBits_t m_PS;
};

class MAINMODULE_EXPORT PicTimer2 : public PicTimer8bit
{
    public:
        PicTimer2( eMcu* mcu, QString name );
        ~PicTimer2();

        virtual void configureA( uint8_t NewT2CON ) override;
        virtual void configureB( uint8_t NewPR2 ) override;

    protected:
        uint8_t m_ps;
        //uint8_t* m_PR2;

        regBits_t m_TMR2ON;
        regBits_t m_T2CKPS;
        regBits_t m_TOUTPS;
};


class MAINMODULE_EXPORT PicTimer16bit : public PicTimer
{
    public:
        PicTimer16bit( eMcu* mcu, QString name );
        ~PicTimer16bit();

    protected:

};

class MAINMODULE_EXPORT PicTimer1 : public PicTimer16bit
{
    public:
        PicTimer1( eMcu* mcu, QString name );
        ~PicTimer1();

        virtual void configureA( uint8_t NewT1CON ) override;

    protected:
        virtual void sheduleEvents() override;

        bool m_t1Osc;

        regBits_t m_T1CKPS;
        regBits_t m_T1OSCEN;

        regBits_t m_TMR1CS;
        regBits_t m_TMR1ON;

};

#endif
