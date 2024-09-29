/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICADC_H
#define PICADC_H

#include "mcuadc.h"
#include "mcutypes.h"

class eMcu;
class McuPin;
class PicTimer80;
class PicTimer16bit;
//class McuOcUnit;

class PicAdc : public McuAdc
{
    public:
        PicAdc( eMcu* mcu, QString name );
        ~PicAdc();

 static PicAdc* createAdc( eMcu* mcu, QString name, int type );

        virtual void setup() override;
        virtual void initialize() override;

        virtual void configureA( uint8_t newADCON0 ) override;
        //virtual void configureB( uint8_t newADCON1 ) override;
        //virtual void callBack() override { if( !m_converting ) startConversion(); }

        virtual void sleep( int mode ) override;

    protected:
        virtual void endConversion() override;
        void setAdcClock( uint8_t prs );

        uint8_t m_mode;

        regBits_t m_ADON;
        regBits_t m_ADSC;
        regBits_t m_CHS;
        regBits_t m_GODO;

        regBits_t m_ADFM;
};

class PicAdc00 : public PicAdc
{
    public:
        PicAdc00( eMcu* mcu, QString name );
        ~PicAdc00();

        virtual void setup() override;
        //virtual void initialize() override;

        //virtual void configureA( uint8_t newADCSRA ) override;
        virtual void configureB( uint8_t newADCSRB ) override;

    protected:
        virtual void updtVref() override;

        regBits_t m_PCFG;
};

class PicAdc1 : public PicAdc
{
    public:
        PicAdc1( eMcu* mcu, QString name );
        ~PicAdc1();

        virtual void setup() override;

        virtual void setANSEL( uint8_t newANSEL );
        void updtANSEL();

    protected:
        virtual void updtVref() override;

        uint8_t* m_ANSEL;
        uint8_t* m_ANSELH;

        regBits_t m_VCFG;
};

class PicAdc10 : public PicAdc1
{
    public:
        PicAdc10( eMcu* mcu, QString name );
        ~PicAdc10();

        virtual void setup() override;

        virtual void configureB( uint8_t newADCSRB ) override;

        void setANSELH( uint8_t newANSELH );
};

class PicAdc11 : public PicAdc1
{
    public:
        PicAdc11( eMcu* mcu, QString name );
        ~PicAdc11();

        virtual void setup() override;

        virtual void configureA( uint8_t newADCON0 ) override;
        //virtual void configureB( uint8_t newADCSRB ) override;
        virtual void setANSEL( uint8_t newANSEL ) override;
};

class PicVrefE;

class PicAdc20 : public PicAdc
{
    public:
        PicAdc20( eMcu* mcu, QString name );
        ~PicAdc20();

        virtual void setup() override;

        virtual void configureA( uint8_t newADCON0 ) override;
        virtual void configureB( uint8_t newADCON1 ) override;

    protected:
        virtual void updtVref() override;

        regBits_t m_ADXREF;

        PicVrefE* m_fvr;
};

class PicAdc3 : public PicAdc
{
    public:
        PicAdc3( eMcu* mcu, QString name );
        ~PicAdc3();

    protected:
};
#endif
