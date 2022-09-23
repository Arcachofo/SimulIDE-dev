/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picadc.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "datautils.h"
#include "regwatcher.h"

PicAdc* PicAdc::createAdc( eMcu* mcu, QString name, int type )
{
    switch( type ){
        case 00: return new PicAdc00( mcu, name ); break;
        case 10: return new PicAdc10( mcu, name ); break;
        case 11: return new PicAdc11( mcu, name ); break;
        case 20: return new PicAdc20( mcu, name ); break;
        default: return NULL;
}   }

PicAdc::PicAdc( eMcu* mcu, QString name )
      : McuAdc( mcu, name )
{
    m_ADON = getRegBits( "ADON", mcu );
    m_GODO = getRegBits( "GO/DONE", mcu );
    m_ADFM = getRegBits( "ADFM", mcu );

    m_pRefPin = NULL;
    m_nRefPin = NULL;
}
PicAdc::~PicAdc(){}

void PicAdc::initialize()
{
    McuAdc::initialize();
    m_pRefPin = m_refPin.at(0);
    if( m_refPin.size() > 1 ) m_nRefPin = m_refPin.at(1);
}

void PicAdc::configureA( uint8_t newADCON0 ) // ADCON0
{
    m_enabled = getRegBitsBool( newADCON0, m_ADON );

    uint8_t prs = getRegBitsVal( newADCON0, m_ADSC );
    if( prs == 3 ) m_convTime = 4*12*1e6;
    else           m_convTime = m_mcu->psCycle()*12*m_prescList[prs];

    m_channel = getRegBitsVal( newADCON0, m_CHS );

    bool convert = getRegBitsBool( newADCON0, m_GODO );
    if( !m_converting && convert ) startConversion();
}

void PicAdc::endConversion()
{
    if( m_leftAdjust ) m_adcValue <<= 6;
    clearRegBits( m_GODO ); // Clear GO/DONE bit
}

//------------------------------------------------------
//-- PIC ADC Type 0 ------------------------------------

PicAdc00::PicAdc00( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
    m_ADSC = getRegBits( "ADSC0,ADCS1", mcu );
    m_CHS  = getRegBits( "CHS0,CHS1,CHS2", mcu );
    m_PCFG = getRegBits( "PCFG0,PCFG1,PCFG2,PCFG3", mcu );
}
PicAdc00::~PicAdc00(){}

void PicAdc00::configureB( uint8_t newADCON1 ) // ADCON1
{
    m_leftAdjust = !getRegBitsBool( newADCON1, m_ADFM );

    uint8_t mode = getRegBitsVal( newADCON1, m_PCFG );
    if( mode != m_mode )
    {
        m_mode = mode;
        uint8_t analog = 0;

        switch( mode ) {
            case 0:  analog = 0b11111111; break;
            case 1:  analog = 0b11110111; break;
            case 2:  analog = 0b00011111; break;
            case 3:  analog = 0b00010111; break;
            case 4:  analog = 0b00001011; break;
            case 5:  analog = 0b00000011; break;
            case 8:  analog = 0b11110011; break;
            case 9:  analog = 0b00111111; break;
            case 10: analog = 0b00110111; break;
            case 11: analog = 0b00110011; break;
            case 12: analog = 0b00010011; break;
            case 13: analog = 0b00000011; break;
            case 14:
            case 15: analog = 0b00000001;
        }
        for( uint i=0; i<m_adcPin.size(); ++i)
            if( m_adcPin[i] ) m_adcPin[i]->setAnalog( analog & (1<<i) );
}   }

void PicAdc00::updtVref()
{
    m_vRefP = 5;
    m_vRefN = 0;

    switch( m_mode ){
        case 1:
        case 3:
        case 5:
        case 10: m_vRefP = m_pRefPin->getVoltage(); break;
        case 8:
        case 11:
        case 12:
        case 13:
        case 15: m_vRefP = m_pRefPin->getVoltage();
                 m_vRefN = m_nRefPin->getVoltage();
}   }

//------------------------------------------------------
//-- PIC ADC Type 1 ------------------------------------

PicAdc1::PicAdc1( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
    m_ANSELH = NULL;
    m_ANSEL  = mcu->getReg( "ANSEL" );
    watchRegNames( "ANSEL" , R_WRITE, this, &PicAdc1::setANSEL , mcu );
}
PicAdc1::~PicAdc1(){}

void PicAdc1::setANSEL( uint8_t newANSEL )
{
    *m_ANSEL = newANSEL;
    updtANSEL();
}

void PicAdc1::updtANSEL()
{
    uint16_t analog = *m_ANSEL;
    if( m_ANSELH ) analog |= (*m_ANSELH << 8);
    for( uint i=0; i<m_adcPin.size(); ++i)
        if( m_adcPin[i] ) m_adcPin[i]->setAnalog( (analog>>i) & 1 );
}

void PicAdc1::updtVref()
{
    m_vRefP = (m_mode & 1) ? m_pRefPin->getVoltage() : 5;
    if( m_nRefPin ) m_vRefN = (m_mode & 0b00000010) ? m_nRefPin->getVoltage() : 0;
}

//------------------------------------------------------
//-- PIC ADC Type 10 p16F88x ---------------------------

PicAdc10::PicAdc10( eMcu* mcu, QString name )
        : PicAdc1( mcu, name )
{
    m_ADSC = getRegBits( "ADSC0,ADCS1", mcu );
    m_CHS  = getRegBits( "CHS0,CHS1,CHS2,CHS3", mcu );
    m_VCFG = getRegBits( "VCFG0,VCFG1", mcu );

    m_ANSELH = mcu->getReg( "ANSELH" );
    watchRegNames( "ANSELH", R_WRITE, this, &PicAdc10::setANSELH, mcu );
}
PicAdc10::~PicAdc10(){}

void PicAdc10::configureB( uint8_t newADCON1 ) // ADCON1
{
    m_leftAdjust = !getRegBitsBool( newADCON1, m_ADFM );
    m_mode       =  getRegBitsVal(  newADCON1, m_VCFG );
}

void PicAdc10::setANSELH( uint8_t newANSELH )
{
    *m_ANSELH = newANSELH;
    updtANSEL();
}

//------------------------------------------------------
//-- PIC ADC Type 11 p12F675 ---------------------------

PicAdc11::PicAdc11( eMcu* mcu, QString name )
        : PicAdc1( mcu, name )
{
    m_ADSC = getRegBits( "ADSC0,ADCS1,ADCS2", mcu );
    m_CHS  = getRegBits( "CHS0,CHS1", mcu );
    m_VCFG = getRegBits( "VCFG", mcu );
}
PicAdc11::~PicAdc11(){}

void PicAdc11::configureA( uint8_t newADCON0 )
{
    m_leftAdjust = !getRegBitsBool( newADCON0, m_ADFM );
    m_mode       = getRegBitsVal(   newADCON0, m_VCFG );
    m_enabled    = getRegBitsBool(  newADCON0, m_ADON );
    m_channel    = getRegBitsVal(   newADCON0, m_CHS );

    bool convert = getRegBitsBool( newADCON0, m_GODO );
    if( !m_converting && convert ) startConversion();
}

void PicAdc11::setANSEL( uint8_t newANSEL )
{
    uint8_t prs = getRegBitsVal( newANSEL, m_ADSC );
    if( prs == 3 ) m_convTime = 4*12*1e6;
    else           m_convTime = m_mcu->psCycle()*12*m_prescList[prs];
    PicAdc1::setANSEL( newANSEL );
}

//------------------------------------------------------
//-- PIC ADC Type 2 p16f1826 ---------------------------

PicAdc20::PicAdc20( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
    m_ADSC = getRegBits( "ADCS0,ADCS1,ADCS2", mcu );
    m_CHS  = getRegBits( "CHS0,CHS1,CHS2,CHS3,CHS4", mcu );
    m_ADXREF = getRegBits( "ADPREF0,ADPREF1,ADNREF", mcu );
}
PicAdc20::~PicAdc20(){}

void PicAdc20::configureA (uint8_t newADCON0 )
{
    m_enabled    = getRegBitsBool( newADCON0, m_ADON );
    m_channel    = getRegBitsVal(  newADCON0, m_CHS );
    bool convert = getRegBitsBool( newADCON0, m_GODO );
    if( !m_converting && convert ) startConversion();
}

void PicAdc20::configureB( uint8_t newADCON1 )
{
    m_leftAdjust = !getRegBitsBool( newADCON1, m_ADFM );

    m_mode = getRegBitsVal( newADCON1, m_ADXREF );

    uint8_t prs = getRegBitsVal( newADCON1, m_ADSC );
    if( prs == 3 ) m_convTime = 4*12*1e6;
    else           m_convTime = m_mcu->psCycle()*12*m_prescList[prs];
}

void PicAdc20::updtVref()
{
    m_vRefP = 5;  // VREF+ is connected to VDD
    switch ( m_mode ) {
    case 2: m_vRefP = m_pRefPin->getVoltage(); break; // VREF+ is connected to external VREF+ pin
    case 3: m_vRefP = 1.024;                break; /// TODO // VREF+ is connected to internal Fixed Voltage Reference (FVR) module
    }
    m_vRefN = (m_mode & 0b00000100) ? m_nRefPin->getVoltage() : 0;
}

//------------------------------------------------------
//-- PIC ADC Type 3 ------------------------------------

PicAdc3::PicAdc3( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
}
PicAdc3::~PicAdc3(){}


