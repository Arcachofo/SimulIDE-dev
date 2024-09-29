﻿/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICTIMER_H
#define PICTIMER_H

#include "mcutimer.h"

class PicTimer : public McuTimer
{
    public:
        PicTimer( eMcu* mcu, QString name );
        ~PicTimer();

 static McuTimer* createTimer( eMcu* mcu, QString name, int type  );

        virtual void initialize() override;

        virtual void configureA( uint8_t val ) override;
        virtual void configureB( uint8_t val ) override;

    protected:
        //virtual void configureClock();
        //void configureExtClock();
};


class PicTimer8bit : public PicTimer
{
    public:
        PicTimer8bit( eMcu* mcu, QString name );
        ~PicTimer8bit();

    protected:
};

class PicTimer0 : public PicTimer8bit
{
    public:
        PicTimer0( eMcu* mcu, QString name );
        ~PicTimer0();

        virtual void setup() override;
        virtual void initialize() override;
        virtual void configureA( uint8_t NewOPTION ) override;

    protected:

        regBits_t m_T0CS;
        regBits_t m_T0SE;
        regBits_t m_PSA;
        regBits_t m_PS;
};

class PicTimer2 : public PicTimer8bit
{
    public:
        PicTimer2( eMcu* mcu, QString name );
        ~PicTimer2();

        virtual void setup() override;

        virtual void configureA( uint8_t NewT2CON ) override;
        virtual void configureB( uint8_t NewPR2 ) override;

    protected:
        uint8_t m_ps;
        //uint8_t* m_PR2;

        regBits_t m_TMR2ON;
        regBits_t m_T2CKPS;
        regBits_t m_TOUTPS;
};


class PicTimer16bit : public PicTimer
{
    public:
        PicTimer16bit( eMcu* mcu, QString name );
        ~PicTimer16bit();

        virtual void setup() override;

        virtual void configureA( uint8_t NewT1CON ) override;

        virtual void sleep( int mode ) override;

    protected:
        virtual void configureClock(){;}
        virtual void sheduleEvents() override;

        bool m_t1Osc;
        uint8_t m_t1sync;

        regBits_t m_T1CKPS;
        regBits_t m_T1OSCEN;
        regBits_t m_T1SYNC;

        regBits_t m_TMR1CS;
        regBits_t m_TMR1ON;
};

class PicTimer160 : public PicTimer16bit
{
    public:
        PicTimer160( eMcu* mcu, QString name );
        ~PicTimer160();

        virtual void setup() override;

    protected:
        virtual void configureClock() override;
};

class PicTimer161 : public PicTimer16bit
{
    public:
        PicTimer161( eMcu* mcu, QString name );
        ~PicTimer161();

        virtual void setup() override;

        //virtual void configureA( uint8_t NewT1CON ) override;

    protected:
        virtual void configureClock() override;
        //virtual void sheduleEvents() override;
};
#endif
