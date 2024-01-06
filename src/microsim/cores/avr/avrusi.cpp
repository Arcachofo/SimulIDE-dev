/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "avrusi.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "avrtimer.h"
#include "mcuocunit.h"
#include "mcuinterrupts.h"
#include "datautils.h"

AvrUsi::AvrUsi( eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , eElement( name )
{
    m_DOpin = NULL;
    m_DIpin = NULL;
    m_CKpin = NULL;

    m_dataReg   = mcu->getReg("USIDR");
    m_bufferReg = mcu->getReg("USIBR");
    m_statusReg = mcu->getReg("USISR");

    m_USITC  = getRegBits("USITC", mcu );
    m_USICLK = getRegBits("USICLK", mcu );
    m_USICS  = getRegBits("USICS0,USICS1", mcu );
    m_USIWM  = getRegBits("USIWM0,USIWM1", mcu );

    m_USICNT = getRegBits("USICNT0,USICNT1,USICNT2,USICNT3", mcu );
    m_USIPF  = getRegBits("USIPF", mcu );

    m_timer0 = (AvrTimer800*)mcu->getTimer("TIMER0");
    m_t0OCA  = m_timer0->getOcUnit("OCA");
    m_t0OCB  = m_timer0->getOcUnit("OCB");
}
AvrUsi::~AvrUsi(){}

void AvrUsi::reset()
{
    m_timer    = false;
    m_extClk   = false;
    m_usiClk   = false;
    m_clkEdge  = false;
    m_clkState = false;
    m_clockMode = 0;
    m_mode = 0;
    m_counter = 0;

    if( !m_DOpin ) qDebug() << "AvrUsi::configureA: Error: null DO Pin";
    if( !m_DIpin ) qDebug() << "AvrUsi::configureA: Error: null DI Pin";
    if( !m_CKpin ) qDebug() << "AvrUsi::configureA: Error: null CK Pin";
}

void AvrUsi::voltChanged()  // Clk Pin changed
{
    bool clk = m_CKpin->getInpState();
    if( m_clkState == clk ) return;

    if( m_mode > 1 ) // TWI start/stop detector
    {
        bool sdaState = m_DIpin->getInpState();
        if( clk  ){
            if     (  m_sdaState && !sdaState) { if( m_startInte ) m_startInte->raise(); }// Start condition
            else if( !m_sdaState &&  sdaState) setRegBits( m_USIPF );                     // Stop condition, set USIPF flag
        }
    }

    if( !m_usiClk ) stepCounter();              // Counter Both edges

    if( ( m_clkEdge && (!m_clkState &&  clk))   // SR Rising  edge
     || (!m_clkEdge && ( m_clkState && !clk)) ) // SR Falling edge
    {
        shiftData();
    }
    m_clkState = clk;
}

void AvrUsi::callBack()  // Called at Timer0 Compare Match
{
    stepCounter();
    shiftData();
}

void AvrUsi::configureA( uint8_t newUSICR )
{
    uint8_t mode = getRegBitsVal( newUSICR, m_USIWM );
    if( m_mode != mode )
    {
        m_mode = mode;

        bool spi = false;
        bool twi = false;

        switch( mode ) {
            case 0:                break; // Disabled
            case 1: spi = true;    break; // 3 Wire mode: Uses DO, DI, and USCK pins.
            case 2:                       // 2 Wire mode: Uses SDA (DI) and SCL (USCK) pins.
            case 3: twi = true;           // Same as 2 wire above & SCL held low at counter overflow
        }

        if( m_DOpin ) m_DOpin->controlPin( spi, false );

        if( twi ) m_sdaState = m_DIpin->getInpState(); // 2 Wire mode: SDA (DI) & SCL (USCK) open collector if DDRB=out, pullups disabled

        if( m_DIpin ){
            m_DIpin->changeCallBack( this, twi );
            m_DIpin->setOpenColl( twi );
        }
        if( m_CKpin ) m_CKpin->setOpenColl( twi );
    }
    if( !m_mode ) return; // Disabled

    uint8_t clockMode = getRegBitsVal( newUSICR, m_USICS );
    if( m_clockMode != clockMode )
    {
        m_clockMode = clockMode;
        m_clkEdge   = false;
        bool extClk = false;
        bool timer  = false;

        switch( clockMode ) {
            case 0:                   break; // Software clock strobe (USICLK)
            case 1:     timer = true; break; // Timer0 Compare Match
            case 2: m_clkEdge = true;        // External, shiftData() positive edge
            case 3:    extClk = true;        // External, shiftData() negative edge
        }
        if( m_extClk != extClk ){            // Activate/Deactivate External Clock
            m_extClk = extClk;
            if( m_CKpin ) m_CKpin->changeCallBack( this, extClk );
        }
        if( m_timer != timer ){
            m_timer = timer;
            m_t0OCA->getInterrupt()->callBack( this, timer );
            m_t0OCB->getInterrupt()->callBack( this, timer );
        }
    }
    if( m_timer ) return;

    bool usiTc = getRegBitsBool( newUSICR, m_USITC ); // toggles the USCK/SCL Pin
    if( usiTc ) toggleClock();

    m_usiClk = getRegBitsBool( newUSICR, m_USICLK );
    if( m_usiClk ){
         if( !m_extClk  ) stepCounter(); // Software counter strobe (USICLK)
         else if( usiTc ) stepCounter(); // Software counter strobe (USITC)
    }
    m_mcu->m_regOverride = newUSICR & 0b11111100; // USICLK & USITC always read as 0
}

void AvrUsi::configureB( uint8_t newUSISR )
{
    m_counter = getRegBitsVal( newUSISR, m_USICNT ); // USICNT[3:0]: Counter Value

    bool oldUsiSR = getRegBitsBool( *m_statusReg, m_USIPF );
    bool newUsiSR = getRegBitsBool(  newUSISR,    m_USIPF );
    if( oldUsiSR && newUsiSR ) m_mcu->m_regOverride = newUSISR & ~m_USIPF.mask; // clear USIPF by writing a 1 to it
}

void AvrUsi::stepCounter()
{
    *m_statusReg = overrideBits( m_counter, m_USICNT ); // increment counter

    if( ++m_counter == 0x0F ){
        m_counter = 0;
        if( m_interrupt ) m_interrupt->raise();
    }
}

void AvrUsi::shiftData()
{
    *m_dataReg = *m_dataReg<<1;                         // Shift data reg

    if( m_DIpin ) // Read input
    {
        bool bit0 = m_DIpin->getInpState();
        if( bit0 ) *m_dataReg |= 1;
        else       *m_dataReg &= ~1;
    }
    if( m_DOpin ) // Set output *m_dataReg & 1<<7
    {
        bool DoState = *m_dataReg & 1<<7;
        if( DoState ) m_mcu->writeReg( m_DObit.regAddr, *m_DObit.reg & ~m_DObit.mask); // Set CLK Low
        else          m_mcu->writeReg( m_DObit.regAddr, *m_DObit.reg |  m_DObit.mask); // Set CLK High
    }
}

void AvrUsi::toggleClock()
{
    if( !m_CKpin ) return;

    bool clkState = getRegBitsBool( m_CKbit );

    if( clkState ) m_mcu->writeReg( m_CKbit.regAddr, *m_CKbit.reg & ~m_CKbit.mask); // Set CLK Low
    else           m_mcu->writeReg( m_CKbit.regAddr, *m_CKbit.reg |  m_CKbit.mask); // Set CLK High
}

void AvrUsi::setPins( QString pinStr )
{
    QStringList pins = pinStr.split(",");
    if( pins.size() < 3 ){
        qDebug() << "AvrUsi::setPins Error:" << pinStr;
        return;
    }
    QString DOpin = pins.value(0);
    QString DIpin = pins.value(1);
    QString CKpin = pins.value(2);

    m_DOpin = m_mcu->getMcuPin( DOpin );
    m_DIpin = m_mcu->getMcuPin( DIpin );
    m_CKpin = m_mcu->getMcuPin( CKpin );

    m_DObit = getRegBits( DOpin, m_mcu );
    m_DIbit = getRegBits( DIpin, m_mcu );
    m_CKbit = getRegBits( CKpin, m_mcu );
}
