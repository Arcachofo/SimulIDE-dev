/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "piccomparator.h"
#include "datautils.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "mcuvref.h"

PicComp* PicComp::createComparator( eMcu* mcu, QString name, int type ) // Static
{
    switch( type ){
        case 01: return new PicComp01( mcu, name );
        case 02: return new PicComp02( mcu, name );
        case 03: return new PicComp03( mcu, name );
        case 11: return new PicComp11( mcu, name );
        case 12: return new PicComp12( mcu, name );
        default: return nullptr;
    }
}

PicComp::PicComp( eMcu* mcu, QString name )
       : McuComp( mcu, name )
{
}
PicComp::~PicComp(){}

void PicComp::setup()
{
    McuVref* vref = m_mcu->vrefModule();
    if( vref ) vref->addCallBack( this, true ); // Vref Module will update vref if changed.
}

void PicComp::initialize()
{
    McuComp::initialize();
    m_pinP = nullptr;
    m_pinN = nullptr;
    m_pinOut = nullptr;
    m_enabled = false;
}

void PicComp::voltChanged()
{
    if( !m_enabled ) return;

    double vRef = m_fixVref ? m_vref : m_pinP->getVoltage();
    bool compOut = vRef > m_pinN->getVoltage() ;
    if( m_inv ) compOut = !compOut;

    if( m_compOut != compOut )
    {
        m_compOut = compOut;
        m_interrupt->raise();
        if( m_pinOut ) m_pinOut->setOutState( m_compOut );

        if( m_compOut ) setRegBits( m_COUT );
        else            clearRegBits( m_COUT );
    }
}

void PicComp::connect( McuPin* pinN, McuPin* pinP, McuPin* pinOut )
{
    if( pinN != m_pinN )
    {
        if( m_pinN ) m_pinN->changeCallBack( this, false );
        if( pinN )   pinN->changeCallBack( this, true );
        m_pinN = pinN;
    }
    if( pinP != m_pinP )
    {
        if( m_pinP ) m_pinP->changeCallBack( this, false );
        if( pinP )   pinP->changeCallBack( this, true );
        m_pinP = pinP;
    }
    if( pinOut != m_pinOut )
    {
        if( m_pinOut ) m_pinOut->controlPin( false, false );
        if( pinOut )   pinOut->controlPin( true, false );

        m_pinOut = pinOut;
    }
    if( m_pinN ) voltChanged(); // Update Comparator state
}

//-------------------------------------------------------------
// Type 0: 16f627 comparators  --------------------------------

PicComp0::PicComp0( eMcu* mcu, QString name )
        : PicComp( mcu, name )
{
}
PicComp0::~PicComp0(){}

void PicComp0::setup()
{
    PicComp::setup();

    m_CM  = getRegBits("CM0,CM1,CM2", m_mcu );
    m_CIS = getRegBits("CIS", m_mcu );
}

void PicComp0::configureA( uint8_t newCMCON )
{
    m_cis = getRegBitsBool( newCMCON, m_CIS );
    m_inv = getRegBitsBool( newCMCON, m_CINV );

    uint8_t mode = getRegBitsVal( newCMCON, m_CM );
    if( mode != m_mode )
    {
        m_mode = mode;
        m_fixVref = false;
        m_enabled = true;
        setMode( mode );
    }
    voltChanged();
}

//-------------------------------------------------------------
// Type 01: 16f627 comparator 1 -------------------------------

PicComp01::PicComp01( eMcu* mcu, QString name )
         : PicComp0( mcu, name )
{
}
PicComp01::~PicComp01(){}

void PicComp01::setup()
{
    PicComp0::setup();

    m_CINV = getRegBits("C1INV", m_mcu );
    m_COUT = getRegBits("C1OUT", m_mcu );
}

void PicComp01::setMode( uint8_t mode )
{
    McuPin* pinN = m_cis ? m_pins[1] : m_pins[0];

    switch ( mode ) {
        case 0: m_enabled = false; connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 1:                    connect( pinN     , m_pins[2], nullptr      ); break;
        case 2: m_fixVref = true;  connect( pinN     , nullptr     , nullptr      ); break;
        case 3:                    connect( m_pins[0], m_pins[2], nullptr      ); break;
        case 4:                    connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 5: m_enabled = false; connect( nullptr     , nullptr     , nullptr      ); break;
        case 6:                    connect( m_pins[0], nullptr     , m_pins[1] ); break;
        case 7: m_enabled = false; connect( nullptr     , nullptr     , nullptr      ); break;
    }
}

//-------------------------------------------------------------
// Type 02: 16f627 comparator 2 -------------------------------

PicComp02::PicComp02( eMcu* mcu, QString name )
         : PicComp0( mcu, name )
{
}
PicComp02::~PicComp02(){}

void PicComp02::setup()
{
    PicComp0::setup();

    m_CINV = getRegBits("C2INV", m_mcu );
    m_COUT = getRegBits("C2OUT", m_mcu );
}

void PicComp02::setMode( uint8_t mode )
{
    McuPin* pinN = m_cis ? m_pins[1] : m_pins[0];

    switch ( mode ) {
        case 0: m_enabled = false;
        case 1:                    connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 2: m_fixVref = true;  connect( pinN     , nullptr     , nullptr      ); break;
        case 3:                                                                // Fallthrough
        case 4:                                                                // Fallthrough
        case 5:                    connect( m_pins[0], m_pins[1], m_pins[2] ); break;
        case 6:                    connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 7: m_enabled = false; connect( nullptr     , nullptr     , nullptr      ); break;
    }
}

//-------------------------------------------------------------
// Type 01: 16f629 comparator ---------------------------------

PicComp03::PicComp03( eMcu* mcu, QString name )
         : PicComp0( mcu, name )
{
}
PicComp03::~PicComp03(){}

void PicComp03::setup()
{
    PicComp0::setup();

    m_CINV = getRegBits("CINV", m_mcu );
    m_COUT = getRegBits("COUT", m_mcu );
}

void PicComp03::setMode( uint8_t mode )
{
    McuPin* pinO = nullptr;
    McuPin* pinN = m_cis ? m_pins[1] : m_pins[0];

    switch ( mode ) {
        case 0: m_enabled = false; connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 1:                    connect( m_pins[0], m_pins[1], m_pins[2] ); break;
        case 2:                    connect( m_pins[0], m_pins[1], nullptr      ); break;
        case 3: m_fixVref = true;  connect( m_pins[0], nullptr     , m_pins[2] ); break;
        case 4: m_fixVref = true;  connect( m_pins[0], nullptr     , nullptr      ); break;
        case 5:                                             pinO = m_pins[2];  // Fallthrough
        case 6: m_fixVref = true;  connect( pinN     , nullptr     , pinO      ); break;
        case 7: m_enabled = false; connect( nullptr     , nullptr     , nullptr      ); break;
    }
}

//-------------------------------------------------------------
// Type 1: 16f88x comparators  --------------------------------

PicComp1::PicComp1( eMcu* mcu, QString name )
        : PicComp( mcu, name )
{
}
PicComp1::~PicComp1(){}

void PicComp1::setup()
{
    PicComp::setup();

    QString n = m_name.right(1); // name="Comp01" => n="1"

    m_CxON = getRegBits("C"+n+"ON", m_mcu );
    m_COUT = getRegBits("C"+n+"OUT", m_mcu );
    m_CxOE = getRegBits("C"+n+"OE", m_mcu );
    m_CINV = getRegBits("C"+n+"POL", m_mcu );
    m_CxR  = getRegBits("C"+n+"R", m_mcu );
    m_CxCH = getRegBits("C"+n+"CH0,C"+n+"CH1", m_mcu );
}

void PicComp1::configureA( uint8_t newCMxCON0 )
{
    if( newCMxCON0 == m_mode ) return;
    m_mode = newCMxCON0;

    m_enabled = getRegBitsBool( newCMxCON0, m_CxON );
    if( !m_enabled )
    {
        connect( nullptr, nullptr, nullptr );
        return;
    }
    bool OE = getRegBitsBool( newCMxCON0, m_CxOE );
    McuPin* pinOut = OE ? m_pins[5] : nullptr;

    m_inv     = getRegBitsBool( newCMxCON0, m_CINV );
    m_fixVref = getRegBitsBool( newCMxCON0, m_CxR );

    uint8_t channel = getRegBitsVal( newCMxCON0, m_CxCH );
    connect( m_pins[channel], m_pins[4], pinOut );
}

//-------------------------------------------------------------
// Type 11: 16f88x comparator 1 -------------------------------

PicComp11::PicComp11( eMcu* mcu, QString name )
         : PicComp1( mcu, name )
{

}
PicComp11::~PicComp11(){}

void PicComp11::configureC( uint8_t newSRCON )
{

}

//-------------------------------------------------------------
// Type 12: 16f88x comparator 2 -------------------------------

PicComp12::PicComp12( eMcu* mcu, QString name )
         : PicComp1( mcu, name )
{

}
PicComp12::~PicComp12(){}

void PicComp12::configureB( uint8_t newCM2CON1 )
{

}

void PicComp12::configureC( uint8_t newSRCON )
{

}

//-------------------------------------------------------------
// Type 20: 16f1826 comparators  ------------------------------

PicComp20::PicComp20( eMcu* mcu, QString name )
         : PicComp( mcu, name )
{

}
PicComp20::~PicComp20(){}

void PicComp20::configureA( uint8_t newCMxCON0 )
{

}

void PicComp20::configureB( uint8_t newCMxCON1 )
{

}
