/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picmssp.h"
#include "picspi.h"
#include "pictwi.h"
#include "e_mcu.h"
#include "datautils.h"

PicMssp::PicMssp( eMcu* mcu, QString name, int type )
       : McuModule( mcu, name )
       , eElement( name )
{
    m_SSPMx = getRegBits( "SSPM0,SSPM1,SSPM2,SSPM3", mcu );
    m_SSPEN = getRegBits( "SSPEN", mcu );
}
PicMssp::~PicMssp(){}

void PicMssp::initialize()
{
    m_mode = 0;
    m_enabled = false;
}

void PicMssp::configureA( uint8_t SSPCON )
{
    bool enabled = getRegBitsBool( SSPCON, m_SSPEN );

    uint8_t mode = getRegBitsVal( SSPCON, m_SSPMx );
    if( m_mode != mode || m_enabled != enabled )
    {
        m_enabled = enabled;
        m_mode = mode;
        spiMode_t spiMode = SPI_OFF;
        twiMode_t twiMode = TWI_OFF;

        switch( m_mode ){
            case 0:                              // SPI Master, clock = FOSC/4
            case 1:                              // SPI Master, clock = FOSC/16
            case 2:                              // SPI Master, clock = FOSC/64
            case 3:{                             // SPI Master, clock = TMR2
                spiMode = SPI_MASTER;
                m_spiUnit->setPrescIndex( mode );
                if( mode == 3 ) {;} /// TODO: SPI TIMER2 callback
            } break;
            case 4:                              // SPI Slave, SS pin enabled
            case 5:{                             // SPI Slave, SS pin disabled
                spiMode = SPI_SLAVE;
                m_spiUnit->m_useSS = !(mode == 5);
            } break;
            case 6: twiMode = TWI_SLAVE;  break; // I2C Slave, 7-bit address
            case 7: twiMode = TWI_SLAVE;  break; // I2C Slave, 10-bit address
            case 8: twiMode = TWI_MASTER; break; // I2C Master, clock = FOSC/(4 *(SSPADD+1))
            case 9:                       break; // Load Mask function
            case 10:                      break; // Reserved
            case 11:                      break; // I2C firmware controlled Master (Slave idle)
            case 12:                      break; // Reserved
            case 13:                      break; // Reserved
            case 14: twiMode = TWI_SLAVE; break; // I2C Slave,  7-bit address, Start & Stop interrupts enabled
            case 15: twiMode = TWI_SLAVE; break; // I2C Slave, 10-bit address, Start & Stop interrupts enabled
        }
        if( enabled ) // First disable, then enable
        {
            if( spiMode == SPI_OFF ) m_spiUnit->setMode( spiMode );
            if( twiMode == TWI_OFF ) m_twiUnit->setMode( twiMode );
            if( spiMode != SPI_OFF ) m_spiUnit->setMode( spiMode );
            if( twiMode != TWI_OFF ) m_twiUnit->setMode( twiMode );
        }else{
            m_spiUnit->setMode( SPI_OFF );
            m_twiUnit->setMode( TWI_OFF );
        }
    }
}

