/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "stm32usart.h"
#include "qemudevice.h"
#include "usarttx.h"
#include "usartrx.h"
#include "iopin.h"
#include "simulator.h"

#define SR_OFFSET 0x00
#define SR_TXE_BIT  7
#define SR_TC_BIT   6
#define SR_RXNE_BIT 5
#define SR_ORE_BIT  3

#define DR_OFFSET 0x04

#define BRR_OFFSET 0x08

#define CR1_OFFSET 0x0c
#define CR1_UE_BIT    13
//#define CR1_M_BIT     12
//#define CR1_PCE_BIT   10
//#define CR1_PS_BIT     9
#define CR1_TXEIE_BIT  7
#define CR1_TCIE_BIT   6
#define CR1_RXNEIE_BIT 5
#define CR1_TE_BIT     3
#define CR1_RE_BIT     2

#define CR2_OFFSET 0x10
#define CR2_STOP_START 12
#define CR2_STOP_MASK 0x00003000

#define CR3_OFFSET 0x14
#define CR3_CTSE_BIT 9
#define CR3_RTSE_BIT 8

#define CR3_DMAR_BIT 0x40
#define CR3_DMAT_BIT 0x80

#define GTPR_OFFSET 0x18


uint8_t uartInterrupt[5] = { 37, 38, 39, 52, 53 };


Stm32Usart::Stm32Usart(QemuDevice* mcu, QString name, int n, uint32_t* clk, uint64_t memStart, uint64_t memEnd )
          : QemuUsart( mcu, name, n, clk, memStart, memEnd )
{
    m_interrupt = uartInterrupt[n];
}
Stm32Usart::~Stm32Usart(){}

void Stm32Usart::reset()
{
    m_DR  = 0;
    m_SR  = 1<<SR_TXE_BIT | 1<<SR_TC_BIT;
    m_divider = 0;
    m_intEnable = 0;
    m_irqLevel = 0;
    m_oreRead = 0;
}

void Stm32Usart::writeRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;
    //qDebug() << "\nStm32Usart::writeRegister"<<QString::number( offset, 16 )<<m_eventValue  ;
    switch( offset )
    {
        case SR_OFFSET:   writeSR(  m_eventValue ); break;
        case DR_OFFSET:   writeDR(  m_eventValue ); break;
        case BRR_OFFSET:  setBaudRate( m_eventValue ); break; //writeBRR( m_eventValue ); break;
        case CR1_OFFSET:  writeCR1( m_eventValue ); break;
        case CR2_OFFSET:  writeCR2( m_eventValue ); break;
        case CR3_OFFSET:  writeCR3( m_eventValue ); break;
        case GTPR_OFFSET: write();                  break;
        default:          write();                  break;
    }
}

void Stm32Usart::readRegister()
{
    uint64_t offset = m_eventAddress - m_memStart;

    if( offset == SR_OFFSET )
    {
        m_oreRead = m_SR & 1<<SR_ORE_BIT; // Clear ORE: SR read followed by a DR read.

        //uint16_t mask = 1<<SR_TXE_BIT | 1<<SR_TC_BIT | 1<<SR_RXNE_BIT | 1<<SR_ORE_BIT;
        //m_SR &= ~mask;
        //m_SR |= s->SR_TXE | s->SR_TC | s->SR_RXNE | s->SR_ORE;

        m_arena->regData = m_SR;
    }
    else if( offset == DR_OFFSET )
    {
        if( m_oreRead ) {      // SR read followed by a DR read.
            m_oreRead = 0;
            m_SR &= ~(1<<SR_ORE_BIT); // Clear ORE
        }
        m_SR &= ~(1<<SR_RXNE_BIT);

        updateIrq();

        m_arena->regData = m_receiver->getData();
    }
    else m_arena->regData = read();

    //qDebug() << "\nStm32Usart::readRegister"<<QString::number( offset, 16 )<<m_arena->regValue ;
    m_arena->qemuAction = SIM_READ;
}

void Stm32Usart::writeCR1( uint16_t newCR1 )
{
    uint16_t enableMask = 1<<CR1_TXEIE_BIT | 1<<CR1_TCIE_BIT | 1<<CR1_RXNEIE_BIT;
    m_intEnable = newCR1 & enableMask;
    //if( m_intEnable & 1<<CR1_RXNEIE_BIT ) s->intFlags |= 1<<SR_ORE_BIT;

    uint8_t enableRx = (newCR1 >> CR1_RE_BIT) & 1;
    uint8_t enableTx = (newCR1 >> CR1_TE_BIT) & 1;
    m_enabled        = (newCR1 >> CR1_UE_BIT) & 1;

    m_receiver->enable( m_enabled && enableRx );
    m_sender->enable(   m_enabled && enableTx );

    writeMem( m_eventAddress, newCR1 & 0x3FFF );

    //qDebug() << "Stm32Usart::writeCR1"<< m_name << m_enabled << enableRx << enableTx;

    updateIrq();
}

void Stm32Usart::writeCR2( uint16_t newCR2 )
{
    writeMem( m_eventAddress, newCR2 & 0x7F7F );
}

void Stm32Usart::writeCR3( uint16_t newCR3 )
{
    //if( newCR3 & CR3_DMAT_BIT ) {
    //    uint64_t curr_time = qemu_clock_get_ns( QEMU_CLOCK_VIRTUAL );
    //    timer_mod( s->tx_dma_timer, curr_time + s->ns_per_char );
    //}

    writeMem( m_eventAddress, newCR3 & 0x07FF );
}

void Stm32Usart::writeSR(  uint16_t newSR )
{
    if( !(newSR & 1<<SR_RXNE_BIT) ) m_SR &= ~(1<<SR_RXNE_BIT); //s->SR_RXNE = 0;
    if( !(newSR & 1<<SR_TC_BIT)   ) m_SR &= ~(1<<SR_TC_BIT);   //s->SR_TC   = 0;

    updateIrq();
}

void Stm32Usart::writeDR(  uint16_t newDR )
{
    //qDebug() <<"Stm32Usart::writeDR"<< m_name << newDR << m_enabled << m_sender->isEnabled();
    /// m_DR = newDR;
    /// if( m_SR & 1<<SR_TC_BIT ) sendNext();                // Transmission Completed, start.
    /// else                      m_SR &= ~(1<<SR_TXE_BIT);  // Mark buffer as not empty

    /// updateIrq();

    /// //writeMem( m_eventAddress, newDR & 0x01FF );

    sendByte( newDR );
}

void Stm32Usart::writeBRR( uint16_t newBRR )
{
    write();

    double fraction = newBRR & 0x000F;
    m_divider = (newBRR>>4) + fraction/16;
    //qDebug() <<"Stm32Usart::writeBRR" << m_name << double(newBRR)/16<<m_BRR;

    freqChanged(); //setBaudRate( *m_frequency/newBRR );
}

void Stm32Usart::sendNext()
{
    //if( m_sender->isEnabled() )
        m_SR &= ~(1<<SR_TC_BIT); // Reset the Transmission Complete flag to indicate a transmit is in progress.
    //qDebug() <<"Stm32Usart::sendNext"<< m_name << m_DR;
    sendByte( m_DR );

    m_SR |= 1<<SR_TXE_BIT;   // Mark transmit buffer empty
}

void Stm32Usart::frameSent( uint8_t data )
{
    UsartModule::frameSent( data );

    //if( m_SR & 1<<SR_TXE_BIT ) { // Buffer is empty
    //    m_SR |= 1<<SR_TC_BIT;    // Set the transmit complete
    //}
    //else {                       // Buffer not empty.
    //    sendNext();              // start transmitting
    //}
    //updateIrq();
}

void Stm32Usart::byteReceived( uint8_t data )
{
    UsartModule::byteReceived( data );


    if( m_SR & 1<<SR_RXNE_BIT ) // Set the overflow flag if buffer not empty
    {
        m_SR |= 1<<SR_ORE_BIT;
        m_oreRead = 0;
    }
    else m_SR |= 1<<SR_RXNE_BIT; // Mark the buffer as not empty.

    updateIrq();

    //if( s->CR3_r & CR3_r_DMAR_BIT )  // DMA operation
    //{
    //    qemu_set_irq(*stm32_DMA1_irq, 0x00000010);
    //    qemu_set_irq(*stm32_DMA1_irq, 0x00000000);
    //}
}

void Stm32Usart::updateIrq() // Called whenever an interrupt-related flag is updated.
{
    uint16_t intFlags = m_SR & (1<<SR_TC_BIT | 1<<SR_TXE_BIT | 1<<SR_RXNE_BIT | 1<<SR_ORE_BIT); // Interupts currently raised
    uint16_t irqLevel = (m_intEnable & intFlags) > 0;       // Filter interrupts enabled

    if( m_irqLevel != irqLevel ) // Only trigger an interrupt if the IRQ level changes.
    {
        m_irqLevel = irqLevel;

        setInterrupt( m_interrupt, irqLevel );
    }
}

void Stm32Usart::freqChanged()
{
    if( !m_divider ) return;
    setBaudRate( *m_frequency/m_divider );
    qDebug() <<"Stm32Usart::freqChanged" << m_name << m_baudRate<<*m_frequency<<m_divider;
}
