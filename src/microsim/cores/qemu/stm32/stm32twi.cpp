/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "stm32twi.h"
#include "qemudevice.h"
#include "iopin.h"
#include "simulator.h"

#define CR1_OFFSET   0x00
#define CR2_OFFSET   0x04
#define OAR1_OFFSET  0x08
#define OAR2_OFFSET  0x0c
#define DR_OFFSET    0x10
#define SR1_OFFSET   0x14
#define SR2_OFFSET   0x18
#define CCR_OFFSET   0x1c
#define TRISE_OFFSET 0x20

// CR1 bits
#define PE_bit        (1<<0)
#define SMBUS_bit     (1<<1)
#define SMBTYPE_bit   (1<<3)
#define ENARB_bit     (1<<4)
#define ENPEC_bit     (1<<5)
#define ENGC_bit      (1<<6)
#define NOSTRETCH_bit (1<<7)
#define START_bit     (1<<8)
#define STOP_bit      (1<<9)
#define ACK_bit       (1<<10)
#define POS_bit       (1<<11)
#define PEC_bit       (1<<12)
#define ALERT_bit     (1<<13)
#define SWRTS_bit     (1<<15)

// CR2 bits
#define ITERREN_bit   (1<<8)
#define ITEVTEN_bit   (1<<9)
#define ITBUFEN_bit   (1<<10)
#define DMAEN_bit     (1<<11)
#define LAST_bit      (1<<12)

// SR1 bits
#define SB_bit        (1<<0)
#define ADDR_bit      (1<<1)
#define BTF_bit       (1<<2)
#define ADD10_bit     (1<<3)
#define STOPF_bit     (1<<4)
#define RxNE_bit      (1<<6)
#define TxE_bit       (1<<7)
#define BERR_bit      (1<<8)
#define ARLO_bit      (1<<9)
#define AF_bit        (1<<10)
#define OVR_bit       (1<<11)
#define PECERR_bit    (1<<12)
#define TIMEOUT_bit   (1<<14)
#define SMBALERT_bit  (1<<15)

// SR2 bits
#define MSL_bit       (1<<0)
#define BUSY_bit      (1<<1)
#define TRA_bit       (1<<2)

static uint16_t SR1_W0_bits =
    BERR_bit
  | ARLO_bit
  | AF_bit
  | OVR_bit
  | PECERR_bit
  | TIMEOUT_bit
  | SMBALERT_bit;

static uint16_t errIrqMask =
    BERR_bit
  | ARLO_bit
  | AF_bit
  | OVR_bit
  | PECERR_bit
  | TIMEOUT_bit
  | SMBALERT_bit;

static uint16_t evtIrqMask0 =
    SB_bit
  | ADDR_bit
  | ADD10_bit
  | STOPF_bit
  | BTF_bit;

static uint16_t evtIrqMask1 =
    TxE_bit
  | RxNE_bit;

static uint8_t evtInterrupt[2] = { 31, 33 };
static uint8_t errInterrupt[2] = { 32, 34 };

Stm32Twi::Stm32Twi(QemuDevice* mcu, QString name, int n, uint32_t* clk, uint64_t memStart, uint64_t memEnd )
        : QemuTwi( mcu, name, n, clk, memStart, memEnd )
{
    m_evtInterrupt = evtInterrupt[n];
    m_errInterrupt = errInterrupt[n];
}
Stm32Twi::~Stm32Twi(){}

void Stm32Twi::reset()
{
    m_CR1  = 0;
    m_CR2  = 0;
    m_OAR1 = 0;
    m_OAR2 = 0;
    m_DR   = 0;
    m_SR1  = 0;
    m_SR2  = 0;
    m_CCR  = 0;
    m_TRISE = 0;
    m_sr1Read = 0;
    m_needStop = 0;
    m_evtIrqLevel = 0;
    m_errIrqLevel = 0;
}

void Stm32Twi::writeRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;
    qDebug() << "Stm32Twi::writeRegister"<< offset << m_eventValue;

    switch (offset) {
    case CR1_OFFSET: writeCR1( m_eventValue ); break;
    case CR2_OFFSET: writeCR2( m_eventValue ); break;
    case DR_OFFSET:  writeDR(  m_eventValue ); break;
    case SR1_OFFSET:
        m_SR1 &= m_eventValue | ~SR1_W0_bits;   // Clear write-0 bits
        break;
    case SR2_OFFSET:  /*Read Only*/            break;

    default:         write();                  break;
    }
    updateIrq();
}

void Stm32Twi::readRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;

    uint64_t value = 0;

    switch( offset ) {
        case CR1_OFFSET: value = m_CR1; break;
        case CR2_OFFSET: value = m_CR2; break;
        case DR_OFFSET:
            if( m_SR1 & BTF_bit ){
                if( m_sr1Read ){
                    m_SR1 &= ~BTF_bit;
                }
                //m_DR = s->RX_BUFF;
            }
            else{
                m_SR1 &= ~RxNE_bit;
                if( !m_needStop ){
                    //m_DR = i2c_recv(s->bus);
                    m_SR1 |= RxNE_bit;
                }
            }
            m_sr1Read = 0;
            value = m_DR;
            if( !m_needStop ) updateIrq();
            break;
        case SR1_OFFSET: value = m_SR1;
            m_sr1Read = 1;
            break;
        case SR2_OFFSET: value = m_SR2;
            if( m_sr1Read ){
                m_SR1 &= ~ADDR_bit;
                updateIrq();
                m_sr1Read = 0;
            }
            break;

        default:
            m_sr1Read = 0;
            value = read();
    }

    //qDebug() <<"Stm32Twi::readRegister"<< offset << value;
    m_arena->regData    = value;
    m_arena->qemuAction = SIM_READ;
}

void Stm32Twi::writeCR1( uint16_t newCR1 )
{
    if( m_CR1 == newCR1 ) return;
    m_CR1 = newCR1;

    bool enabled = newCR1 & PE_bit;  // Bit 0 PE:        Peripheral enable
    if( m_enabled != enabled )
    {
        if( enabled )              /// Enable TWI
        {
            setMode( TWI_MASTER );        // FIXME
            m_sda->setPinMode( openCo );
            //m_sda->controlPin( true, true ); // Get control of MCU PIns
            m_scl->setPinMode( openCo );
            //m_scl->controlPin( true, true );

        }
        else                      /// Disable TWI
        {
            setMode( TWI_OFF );
            //m_sda->controlPin( false, false ); // Release control of MCU PIns
            //m_scl->controlPin( false, false );

            m_SR1 = 0;
            m_SR2 = 0;
            m_CR1 &= ~START_bit;
        }
        m_enabled = enabled;
        if( !enabled ) return;               // Not enabled, do nothing
    }
                                    // Bit 1 SMBUS:     SMBus mode
                                    // Bit 2            Reserved, must be kept at reset value
                                    // Bit 3 SMBTYPE:   SMBus type
                                    // Bit 4 ENARP:     ARP enable
                                    // Bit 5 ENPEC:     PEC enable
    m_genCall = newCR1 & ENGC_bit;  // Bit 6 ENGC:      General call enable
                                    // Bit 7 NOSTRETCH: Clock stretching disable (Slave mode)
    if( newCR1 & START_bit )        // Bit 8 START:     Start generation
    {
        /// TODO: if transmitting, shedule a restart
        /// if( m_SR2 & MSL_bit ) m_needStart = true;

        qDebug() << "Stm32Twi::writeCR1 Start";
        m_CR1 &= ~START_bit;

        //m_SR2 |= MSL_bit;
        m_SR2 |= BUSY_bit;

        if( m_mode == TWI_MASTER ) masterStart();
    }
    if( newCR1 & STOP_bit )         // Bit 9 STOP: Stop generation
    {
        if( m_mode == TWI_MASTER )
        {
            if( m_SR1 & BTF_bit ) masterStop();   // Stop now
            else                  m_needStop = 1; // Stop after current operation.
        }
        qDebug() << "Stm32Twi::writeCR1 Stop" << m_needStop;
    }

    m_sendACK = newCR1 & ACK_bit;   // Bit 10 ACK:      Acknowledge enable

    /// if( m_sendACK )
    /// {
    ///     if( (m_SR2 & MSL_bit) && !(m_SR2 & TRA_bit) )
    ///     {
    ///         if( !(m_SR1 & RxNE_bit) ) {
    ///             /// m_DR = i2c_recv(s->bus);
    ///             m_SR1 |= RxNE_bit;
    ///         }
    ///         else if( !(m_SR1 & BTF_bit) ){
    ///             /// s->RX_BUFF = i2c_recv(s->bus);
    ///             m_SR1 |= BTF_bit;
    ///         }
    ///     }
    /// }
                                    // Bit 11 POS:      Acknowledge/PEC Position (for data reception)
                                    // Bit 12 PEC:      Packet error checking
                                    // Bit 13 ALERT:    SMBus alert
                                    // Bit 14           Reserved, must be kept at reset value
    if( newCR1 & SWRTS_bit )        // Bit 15 SWRST:    Software reset
    {
        // Reset
    }
    if( m_mode == TWI_MASTER ) m_SR2 |=  MSL_bit;
    else                       m_SR2 &= ~MSL_bit;

}

void Stm32Twi::writeCR2( uint16_t newCR2 )
{
    //if( m_CR2 == newCR2 ) return;
    m_CR2 = newCR2;

    // Bit 10 ITBUFEN: Buffer interrupt enable
    // Bit 9  ITEVTEN: Event  interrupt enable
    // Bit 8  ITERREN: Error  interrupt enable

    double freqMHz = newCR2 & 0b111111;   // Bits 5:0 FREQ[5:0]: Peripheral clock frequency
    if( freqMHz < 2 ) freqMHz = 2;        // Minimum allowed frequency is 2 MHz

    if( m_freq == freqMHz*1e6 ) return;
    TwiModule::setFreqKHz( freqMHz*1000.0 );
    qDebug() << "Stm32Twi::writeCR2 Frequency:" << freqMHz << "MHz";
}

void Stm32Twi::writeDR( uint16_t newDR )
{
    if( m_SR1 & SB_bit )          // Start sent, send Address
    {
        if( !m_sr1Read ) return;
        m_sr1Read = 0;
        m_SR1 &= ~SB_bit;

        qDebug() << "Stm32Twi::writeDR send Address";
        bool write = (newDR & 1) == 0;
        if( write ){             // I2C_START_WRITE
            m_SR2 |= TRA_bit;
            m_SR1 &= ~RxNE_bit;
        }else{                   // I2C_START_READ
            m_SR2 &= ~TRA_bit;
            m_SR1 |= RxNE_bit;
        }
        masterWrite( newDR, true, write );
    }
    else // Address sent & SR1-read+SR2-read: send data
    {
        qDebug() << "Stm32Twi::writeDR send Data";
        m_nextData = 0;
        if( m_SR1 & TxE_bit )       // Data Reg empty: send data
        {
            //m_SR1 &= ~RxNE_bit; // ????
            masterWrite( newDR, false, true ); // I2C_WRITE
        }
        else m_nextData = 1;
        m_DR = newDR;
    }
}

void Stm32Twi::i2cStop()
{
    m_CR1 &= ~STOP_bit;
    m_SR1 &= ~TxE_bit;
    m_SR1 &= ~BTF_bit;
    m_SR2 &= ~MSL_bit;
    m_SR2 &= ~BUSY_bit;
    m_SR2 &= ~TRA_bit;
    m_needStop = 0;

    qDebug() << "Stm32Twi::i2cStop -------------------------------------"<<m_SR1<<m_SR2;
}

void Stm32Twi::setTwiState( twiState_t state )
{
    TwiModule::setTwiState( state );
    qDebug() << "Stm32Twi::setTwiState"<< state;
    uint8_t ackT = 1;
    uint8_t ackR = 1;

    switch( state )
    {              // MASTER
        case TWI_START    :                              // START transmitted
            //fall through
        case TWI_REP_START:                              // Repeated START transmitted
            m_CR1 &= ~START_bit;
            m_SR1 &= ~TxE_bit;
            m_SR1 &= ~BTF_bit;
            m_SR1 |= SB_bit;
            if( state == TWI_REP_START ) m_SR2 &= ~TRA_bit;
            break;
        case TWI_MTX_ADR_ACK  : ackT = 0;                // SLA+W transmitted, ACK  received
            m_SR1 |= ADDR_bit;
            //fall through
        case TWI_MTX_ADR_NACK :                          // SLA+W transmitted, NACK received
            m_SR1 |= TxE_bit;
            break;
        case TWI_MTX_DATA_ACK : ackT = 0;                // Data transmitted, ACK  received
            m_SR1 |= TxE_bit;
            //fall through
        case TWI_MTX_DATA_NACK:                          // Data transmitted, NACK received
            if( m_nextData ) masterWrite( m_DR, false, true ); // I2C_WRITE
            else m_SR1 |= BTF_bit;
            break;
        case TWI_MRX_ADR_ACK  : ackT = 0;                // SLA+R transmitted, ACK  received
            m_SR1 |= ADDR_bit;
            //fall through
        case TWI_MRX_ADR_NACK :                          // SLA+R transmitted, NACK received
            break;
        case TWI_MRX_DATA_ACK : ackR = 0;                // Data received, ACK  returned
             //fall through
        case TWI_MRX_DATA_NACK:                          // Data received, NACK returned
            m_DR = m_rxReg;
            m_SR1 |= BTF_bit;
            break;

                  // SLAVE
        case TWI_SRX_ADR_ACK      : ackR = 0; break;     // Own SLA+W received, ACK returned
        case TWI_SRX_GEN_ACK      : ackR = 0; break;     // General call received, ACK returned
        case TWI_SRX_ADR_DATA_ACK : ackR = 0; break;     // data received, ACK returned
        case TWI_SRX_ADR_DATA_NACK:                      // data received, NACK returned
            m_DR = m_rxReg;
            break;
        case TWI_SRX_GEN_DATA_ACK : ackR = 0; break;     // general call; data received, ACK  returned
        case TWI_SRX_GEN_DATA_NACK:                ;     // general call; data received, NACK returned
            m_DR = m_rxReg;
            break;

        case TWI_STX_ADR_ACK      : ackR = 0; break;     // Own SLA+R received, ACK returned
        case TWI_STX_DATA_ACK     : ackT = 0;            // Data transmitted, ACK received
        case TWI_STX_DATA_NACK    :           break;     // Data transmitted, NACK received

        case TWI_NO_STATE:         i2cStop(); break;    // STOP transmitted, Transmission ended
        default: break;
    }
    if( m_needStop ) masterStop();

    updateIrq();

    /// TODO: implement ACK Error
}

void Stm32Twi::updateIrq()
{
    if( m_CR2 & ITERREN_bit )
    {
        int errIrqLevel = m_SR1 & errIrqMask;

        if( m_errIrqLevel != errIrqLevel ){
            m_errIrqLevel = errIrqLevel;
            setInterrupt( m_errInterrupt, errIrqLevel );
            qDebug() << "Stm32Twi::updateIrq err" << m_errInterrupt << errIrqLevel;
        }
    }

    /// FIXME: solve interrupt overlap

    if( m_CR2 & ITEVTEN_bit)
    {
        int evtIrqLevel =  m_SR1 & evtIrqMask0;

        if( m_CR2 & ITBUFEN_bit)
            evtIrqLevel |= m_SR1 & evtIrqMask1;

        if( m_evtIrqLevel != evtIrqLevel ){
            m_evtIrqLevel = evtIrqLevel;
            setInterrupt( m_evtInterrupt, evtIrqLevel );
            qDebug() << "Stm32Twi::updateIrq evt" << m_evtInterrupt << evtIrqLevel;
        }
    }
}
