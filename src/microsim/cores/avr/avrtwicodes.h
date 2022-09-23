/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/


#ifndef TWICODES_H
#define TWICODES_H

/// TODO: this should dissapear once we use our own TWI Module
enum {
    TWI_COND_START = (1 << 0),
    TWI_COND_STOP  = (1 << 1),
    TWI_COND_ADDR  = (1 << 2),
    TWI_COND_ACK   = (1 << 3),
    TWI_COND_WRITE = (1 << 4),
    TWI_COND_READ  = (1 << 5),
    TWI_COND_SLAVE = (1 << 6), // internal state, do not use in irq messages
    TWI_MSG        = (1 << 7), // for simulide AvrCompBase
};


/*
 * This block respectfully nicked straight out from the Atmel sample
 * code for AVR315. Typos and all.
 * There is no copyright notice on the original file.
 */
/****************************************************************************
  TWI State codes
****************************************************************************/
enum twiState_t{
    // General TWI Master status codes
    TWI_START                  = 0x08, //* 008 START has been transmitted
    TWI_REP_START              = 0x10, // 016 Repeated START has been transmitted
    TWI_ARB_LOST               = 0x38, // 056 Arbitration lost

    // TWI Master Transmitter status codes
    TWI_MTX_ADR_ACK            = 0x18, //* 024 SLA+W has been transmitted and ACK received
    TWI_MTX_ADR_NACK           = 0x20, //* 032 SLA+W has been transmitted and NACK received
    TWI_MTX_DATA_ACK           = 0x28, //* 040 Data byte has been transmitted and ACK received
    TWI_MTX_DATA_NACK          = 0x30, //* 048 Data byte has been transmitted and NACK received

    // TWI Master Receiver status codes
    TWI_MRX_ADR_ACK            = 0x40, //* 064 SLA+R has been transmitted and ACK received
    TWI_MRX_ADR_NACK           = 0x48, //* 072 SLA+R has been transmitted and NACK received
    TWI_MRX_DATA_ACK           = 0x50, //* 080 Data byte has been received and ACK transmitted
    TWI_MRX_DATA_NACK          = 0x58, //* 088 Data byte has been received and NACK transmitted

    // TWI Slave Receiver status codes
    TWI_SRX_ADR_ACK            = 0x60, //* 096 Own SLA+W has been received ACK has been returned
    TWI_SRX_ADR_ACK_M_ARB_LOST = 0x68, // 104 Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
    TWI_SRX_GEN_ACK            = 0x70, //* 112 General call address has been received; ACK has been returned
    TWI_SRX_GEN_ACK_M_ARB_LOST = 0x78, // 120 Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
    TWI_SRX_ADR_DATA_ACK       = 0x80, //* 128 Previously addressed with own SLA+W; data has been received; ACK has been returned
    TWI_SRX_ADR_DATA_NACK      = 0x88, //* 136 Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
    TWI_SRX_GEN_DATA_ACK       = 0x90, //* 144 Previously addressed with general call; data has been received; ACK has been returned
    TWI_SRX_GEN_DATA_NACK      = 0x98, //* 152 Previously addressed with general call; data has been received; NOT ACK has been returned
    TWI_SRX_STOP_RESTART       = 0xA0, // 160 A STOP condition or repeated START condition has been received while still addressed as Slave

    // TWI Slave Transmitter status codes
    TWI_STX_ADR_ACK            = 0xA8, //* 168 Own SLA+R has been received; ACK has been returned
    TWI_STX_ADR_ACK_M_ARB_LOST = 0xB0, // 176 Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
    TWI_STX_DATA_ACK           = 0xB8, //* 184 Data byte in TWDR has been transmitted; ACK has been received
    TWI_STX_DATA_NACK          = 0xC0, //* 192 Data byte in TWDR has been transmitted; NOT ACK has been received
    TWI_STX_DATA_ACK_LAST_BYTE = 0xC8, // 200 Last data byte in TWDR has been transmitted (TWEA = �0�); ACK has been received

    // TWI Miscellaneous status codes
    TWI_NO_STATE               = 0xF8, //* 248 No relevant state information available; TWINT = �0�
    TWI_BUS_ERROR              = 0x00, // 000 Bus error due to an illegal START or STOP condition
};
#endif
