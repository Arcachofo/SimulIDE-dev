/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "pictwi.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "datautils.h"

PicTwi::PicTwi( eMcu* mcu, QString name )
      : McuTwi( mcu, name )
{
    // SSPCON
    m_WCOL    = getRegBits("WCOL"   , mcu );

    // SSPCON2
    m_CGEN    = getRegBits("CGEN"   , mcu );
    m_ACKSTAT = getRegBits("ACKSTAT", mcu );
    m_ACKDT   = getRegBits("ACKDT"  , mcu );
    m_ACKEN   = getRegBits("ACKEN"  , mcu );
    m_RCEN    = getRegBits("RCEN"   , mcu );
    m_PEN     = getRegBits("PEN"    , mcu );
    m_RSEN    = getRegBits("RSEN"   , mcu );
    m_SEN     = getRegBits("SEN"    , mcu );

    //SSPSTAT
    m_P  = getRegBits("P" , mcu );
    m_S  = getRegBits("S" , mcu );
    m_RW = getRegBits("RW", mcu );
    m_BF = getRegBits("BF", mcu );
}
PicTwi::~PicTwi(){}

void PicTwi::initialize()
{
    McuTwi::initialize();
    m_bitRate = 0;
    m_enabled = false;
}

void PicTwi::setMode( twiMode_t mode )
{
    if( m_mode == mode ) return;

    bool oldEn = m_enabled;
    m_enabled  = mode != TWI_OFF;
    if( oldEn && !m_enabled )                 /// Disable TWI
    {
        m_sda->controlPin( false, false ); // Release control of MCU PIns
        m_scl->controlPin( false, false );
    }
    else if( !oldEn && m_enabled )            /// Enable TWI if it was disabled
    {
        m_sda->setPinMode( openCo );
        m_sda->controlPin( true, false ); // Get control of MCU PIns
        m_scl->setPinMode( openCo );
        m_scl->controlPin( true, false );
    }
    TwiModule::setMode( mode );
}

void PicTwi::configureA( uint8_t newSSPCON )
{
    /*bool wcol  = getRegBitsBool( newSSPCON, m_WCOL );
    bool oldWcol = getRegBitsBool( m_WCOL );
    wcol=false;*/
}

void PicTwi::configureB( uint8_t newSSPCON2 )
{
    m_genCall = getRegBitsBool( newSSPCON2, m_CGEN ); /// Interrupt??

    bool newStart  = getRegBitsBool( newSSPCON2, m_SEN )
                   | getRegBitsBool( newSSPCON2, m_RSEN );
    if( newStart )              /// Generate Start Condition
    {
        if( m_mode == TWI_MASTER )
        {
            clearRegBits( m_RW );
            masterStart();
        }
    }

    bool newStop = getRegBitsBool( newSSPCON2, m_PEN );
    if( newStop  )              /// Generate Stop Condition
    {
        if( m_mode == TWI_MASTER ) // Master: Stop if I2C was started
        {
            if( getStaus() < TWI_NO_STATE )
            {
                clearRegBits( m_RW );
                masterStop();
            }
        }
        /// else setMode( TWI_SLAVE ); // Slave: Stop Cond restarts Slave mode (can be used to recover from an error condition)
    }
    if( m_mode == TWI_MASTER )
    {
        bool rcen  = getRegBitsBool( newSSPCON2, m_RCEN );
        if( rcen )
        {
            m_sendACK   = !getRegBitsBool( newSSPCON2, m_ACKDT ); // ACK State to send
            m_masterACK = getRegBitsBool( newSSPCON2, m_ACKEN ); // Send ACK action

            //if( m_twiState == TWI_MRX_ADR_ACK    // We sent Slave Address + R and received ACK
            // || m_twiState == TWI_MRX_DATA_ACK ) // We sent data and received ACK
            if( m_i2cState == I2C_IDLE ) masterRead( m_sendACK );     // Read a byte and send ACK/NACK
            if( m_masterACK && !m_sendACK ) clearRegBits( m_RW );
        }
    }
}

void PicTwi::writeAddrReg( uint8_t newSSPADD )
{
    //if( m_mode == TWI_MASTER ) // Baudrate
    {
        double freq = m_mcu->freq()/(4*(newSSPADD+1));
        setFreqKHz( freq/1e3 );
    }
    //else
        m_address = newSSPADD >> 1; // SLAVE Address
}

void PicTwi::writeStatus( uint8_t newSSPSTAT )
{
    /// Done by masking //m_mcu->m_regOverride = newTWSR | (*m_statReg & 0b11111100); // Preserve Status bits
}

void PicTwi::writeTwiReg( uint8_t newSSPBUF ) // SSPBUF is being written
{
    if( m_mode != TWI_MASTER ) return;

    if( getRegBitsBool( m_BF ) )  // Check if BF is set (still transmitting)
    {
        setRegBits( m_WCOL );  // set Write Collision bit WCOL
        return;                // The access will be discarded
    }
    bool write = false;
    bool isAddr = getRegBitsBool( m_S ); // We just sent Start, so this must be slave address

    if( isAddr )
    {
        write = ((newSSPBUF & 1) == 0); // Sendind address for Read or Write?
        writeRegBits( m_RW, !write );
    }
    setRegBits( m_BF );

    masterWrite( newSSPBUF, isAddr, write );       /// Write data or address to Slave
}

void PicTwi::setTwiState( twiState_t state )  // Set new Status value
{
    clearRegBits( m_S );
    clearRegBits( m_P );

    if( state == TWI_START )                                           // Start sent
    {
        clearRegBits( m_SEN );
        clearRegBits( m_RSEN );
        setRegBits( m_S );
        m_interrupt->raise();
    }
    else if( state == TWI_NO_STATE && m_i2cState == I2C_STOP  )        // Stop sent
    {
        clearRegBits( m_PEN );
        setRegBits( m_P );
        m_interrupt->raise();
    }
    else if( state == TWI_MTX_ADR_ACK  || state == TWI_MTX_ADR_NACK    // Master Addr transmitted
          || state == TWI_MTX_DATA_ACK || state == TWI_MTX_DATA_NACK ) // Master Data transmitted
    {
        if( state == TWI_MTX_ADR_ACK
         || state == TWI_MTX_DATA_ACK ) clearRegBits( m_ACKSTAT );
        else                            setRegBits( m_ACKSTAT );

        m_interrupt->raise();
    }
    else if( state == TWI_MRX_DATA_ACK || state == TWI_MRX_DATA_NACK ) // Master Data received
    {
        //*m_dataReg = m_rxReg; // Save data received into SSPBUF  // Done in readByte()
        clearRegBits( m_ACKEN );
    }
    else if( state == TWI_SRX_ADR_DATA_ACK || state == TWI_SRX_ADR_DATA_NACK   //Slave Addr received
          || state == TWI_SRX_GEN_DATA_ACK || state == TWI_SRX_GEN_DATA_NACK ) //Slave Data received
    {
        //*m_dataReg = m_rxReg; // Save data received into SSPBUF // Done in readByte()
    }

    TwiModule::setTwiState( state );
}

void PicTwi::writeByte() // Read from Data Register
{
    if( m_mode == TWI_SLAVE ) m_txReg = *m_dataReg;
    TwiModule::writeByte();
}

void PicTwi::bufferEmpty() // Data/Addr transmitted (before ACK)
{
    clearRegBits( m_BF );
}

void PicTwi::readByte()
{
    *m_dataReg = m_rxReg; // Save data received into SSPBUF
    setRegBits( m_BF );
    m_interrupt->raise();
    TwiModule::readByte();
}

void PicTwi::readTwiReg( uint8_t val )
{
    clearRegBits( m_BF );
}
