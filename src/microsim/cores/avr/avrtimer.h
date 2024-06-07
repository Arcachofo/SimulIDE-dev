/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRTIMER_H
#define AVRTIMER_H

#include "mcutimer.h"

enum wgmMode_t{
    wgmNORM = 0,
    wgmPHAS,
    wgmCTC,
    wgmFAST,
};

class AvrTimer : public McuTimer
{
    friend class McuCreator;

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
        virtual void updtWgm(){;}
        virtual void configureClock();
        void configureOcUnits( bool wgm3 );
        void updtPrescaler( uint8_t val );

        regBits_t m_WGM10;
        regBits_t m_WGM32;

        wgmMode_t m_wgmMode;
        uint8_t m_wgm10Val;
        uint8_t m_wgm32Val;

        McuOcUnit* m_OCA;
        McuOcUnit* m_OCB;
        McuOcUnit* m_OCC;
};

class AvrTimer8bit : public AvrTimer
{
    public:
        AvrTimer8bit( eMcu* mcu, QString name );
        ~AvrTimer8bit();

        virtual void topReg0Changed( uint8_t val ) override;

    protected:
        virtual void updtWgm() override;

};

class AvrTimer800 : public AvrTimer8bit
{
    public:
        AvrTimer800( eMcu* mcu, QString name );
        ~AvrTimer800();
};

class AvrTimer801 : public McuTimer
{
    public:
        AvrTimer801( eMcu* mcu, QString name );
        ~AvrTimer801();

        virtual void initialize() override;

        virtual void configureA( uint8_t newTCCR0 ) override;

    protected:
        void configureClock();
};

class AvrTimer810 : public AvrTimer
{
    public:
        AvrTimer810( eMcu* mcu, QString name );
        ~AvrTimer810();

        virtual void configureA( uint8_t newTCCR1 ) override;
        virtual void configureB( uint8_t newGTCCR ) override;

        virtual void topReg0Changed( uint8_t val ) override;

    protected:
        void updateMode();
        void updateOcUnit( McuOcUnit*ocUnit, bool pwm );

        regBits_t m_CTC1;
        regBits_t m_PWM1A;
        regBits_t m_PWM1B;
        regBits_t m_PSR1;

        McuPin* m_oc1AiPin;
        McuPin* m_oc1BiPin;
};

class AvrTimer820 : public AvrTimer8bit
{
    public:
        AvrTimer820( eMcu* mcu, QString name );
        ~AvrTimer820();
};

class AvrTimer821 : public AvrTimer8bit
{
    public:
        AvrTimer821( eMcu* mcu, QString name );
        ~AvrTimer821();

        virtual void configureA( uint8_t newTCCRx ) override;
};


class AvrTimer16bit : public AvrTimer
{
    public:
        AvrTimer16bit( eMcu* mcu, QString name );
        ~AvrTimer16bit();

        virtual void runEvent() override;

        virtual void topReg0Changed( uint8_t val ) override;
        void ICRXLchanged( uint8_t val );
        //void ICRXHchanged( uint8_t val );

    protected:
        virtual void updtWgm() override;
        void setICRX( QString reg );

        bool m_useICR;

        uint8_t* m_topReg1L;
        uint8_t* m_topReg1H;
};

#endif
