/*
    avr_twi.c

    Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

     This file is part of simavr.

    simavr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    simavr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "avr_twi.h"
#include "avrtwicodes.h"

#define AVR_TWI_DEBUG 1

static inline void _avr_twi_status_set( avr_twi_t* p, uint8_t v, int interrupt )
{
    avr_regbit_setto_raw( p->io.avr, p->twsr, v );

    avr_raise_irq( p->io.irq + TWI_IRQ_STATUS, v );

    if( interrupt ) avr_raise_interrupt( p->io.avr, &p->twi );
}

static __attribute__ ((unused)) inline uint8_t _avr_twi_status_get( avr_twi_t* p)
{
    return avr_regbit_get_raw( p->io.avr, p->twsr );
}

static void avr_twi_write( struct avr_t* avr, avr_io_addr_t addr, uint8_t v, void* param )
{
    avr_twi_t* p = (avr_twi_t*)param;

    uint8_t twen  = avr_regbit_get( avr, p->twen );
    uint8_t twsta = avr_regbit_get( avr, p->twsta );
    uint8_t twsto = avr_regbit_get( avr, p->twsto );
    uint8_t twint = avr_regbit_get( avr, p->twi.raised );

    //if( twint ) v &= ~(1<<7); // Clear TWINT flag
    avr_core_watch_write( avr, addr, v );

    if( twen != avr_regbit_get(avr, p->twen) )
    {
        twen = !twen;
        if( !twen )  // if we were running, now we are not
        {
            avr_regbit_clear( avr, p->twea );
            avr_regbit_clear( avr, p->twsta );
            avr_regbit_clear( avr, p->twsto );

            avr_clear_interrupt( avr, &p->twi );
            avr_core_watch_write( avr, p->r_twdr, 0xff );
            _avr_twi_status_set( p, TWI_NO_STATE, 0 );

            p->state     = 0;
            p->peer_addr = 0;
        }
        if( avr->data[p->r_twar] )  p->state |= TWI_COND_SLAVE;
    }
    if( !twen ) return;

    uint8_t cleared = avr_regbit_get(avr, p->twi.raised);

    /*int cleared = */
    avr_clear_interrupt_if( avr, &p->twi, twint );

    if( cleared ) avr_regbit_setto( avr, p->twi.raised, 0 ); // Clear TWINT flag

    if( !twsto && avr_regbit_get(avr, p->twsto)) // generate a stop condition
    {
        if( p->state ) // doing stuff
        {
            if( p->state & TWI_COND_START )
            {
                avr_raise_irq(p->io.irq + TWI_IRQ_OUTPUT, avr_twi_irq_msg(TWI_COND_STOP, p->peer_addr, 1));
            }
        }
    }
    if( !twsta && avr_regbit_get(avr, p->twsta) ) // generate a start condition
    {
        uint32_t msg = avr_twi_irq_msg( TWI_COND_START, p->peer_addr, 0 );
        avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT, msg );
    }

    int data = cleared && !avr_regbit_get(avr, p->twsta) && !avr_regbit_get(avr, p->twsto);

    if( !data ) return;

    int do_read = p->peer_addr & 1;
    int do_ack = avr_regbit_get(avr, p->twea) != 0;

    if( p->state & TWI_COND_SLAVE )
    {
        if( p->state & TWI_COND_ADDR ) // writing or reading a byte
        {
            if( do_read )
            {
                if( p->state & TWI_COND_WRITE )
                {
                    avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT,
                        avr_twi_irq_msg( TWI_COND_READ | TWI_COND_ACK, p->peer_addr, avr->data[p->r_twdr]));
                }
            }
            else
            {
                avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT,
                               avr_twi_irq_msg( TWI_COND_ACK, p->peer_addr, 0 ));
            }
            avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT,
                    avr_twi_irq_msg( TWI_COND_ADDR + (do_ack ? TWI_COND_ACK : 0)
                                   , p->peer_addr
                                   , avr->data[p->r_twdr]));
        }
        else                            // address, acknowledge it
        {
            p->state |= TWI_COND_ADDR;
            avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT
                         , avr_twi_irq_msg( TWI_COND_ADDR
                                            | (do_ack ? TWI_COND_ACK : 0)
                                            | (p->state & TWI_COND_WRITE ? TWI_COND_READ : 0)
                                          , p->peer_addr
                                          , avr->data[p->r_twdr]));
        }
    }
    else
    {
        if( p->state & TWI_COND_ADDR ) // writing or reading a byte
        {
            // a normal data byte
            uint8_t msgv = do_read ? TWI_COND_READ : TWI_COND_WRITE;

            if( do_ack ) msgv |= TWI_COND_ACK;

            p->state &= ~TWI_COND_ACK;    // clear ACK bit

            if( p->state & msgv ) // if the latch is ready... as set by writing/reading the TWDR
            {
                uint32_t irqMsg = avr_twi_irq_msg( msgv, p->peer_addr, avr->data[p->r_twdr]);
                avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT, irqMsg );
            }
        }
        else if( p->state ) // send the address
        {
            //p->state |= TWI_COND_ADDR;
            p->peer_addr = avr->data[p->r_twdr];
            p->state &= ~TWI_COND_ACK;    // clear ACK bit

            // we send an IRQ and we /expect/ a slave to reply
            // immediately via an IRQ to set the COND_ACK bit
            // otherwise it's assumed it's been nacked...
            uint32_t irqMsg = avr_twi_irq_msg( TWI_COND_ADDR, p->peer_addr, 0 );

            avr_raise_irq( p->io.irq + TWI_IRQ_OUTPUT, irqMsg );

            if( p->peer_addr & 1 )  // read ?
            {
                p->state |= TWI_COND_READ;    // always allow read to start with
            }
        }
        p->state &= ~TWI_COND_WRITE;
    }
}

/*
 * Write data to the latch, tell the system we have something
 * to send next
 */
static void avr_twi_write_data( struct avr_t* avr, avr_io_addr_t addr, uint8_t v, void* param)
{
    avr_twi_t* p = (avr_twi_t*)param;

    avr_core_watch_write(avr, addr, v);

    p->state |= TWI_COND_WRITE;          // tell system we have something in the write latch
}

/*
 * Read data from the latch, tell the system can receive a new byte
 */
static uint8_t avr_twi_read_data( struct avr_t* avr, avr_io_addr_t addr, void* param)
{
    avr_twi_t* p = (avr_twi_t*)param;

    p->state |= TWI_COND_READ;          // tell system we can receive another byte
    return avr->data[p->r_twdr];
}

/*
 * prevent code from rewriting out status bits, since we actually use them!
 */
static void avr_twi_write_status( struct avr_t* avr, avr_io_addr_t addr, uint8_t v, void* param )
{
    avr_twi_t* p = (avr_twi_t*)param;
    uint8_t sr = avr_regbit_get( avr, p->twsr);
    uint8_t c  = avr_regbit_get( avr, p->twps);

    avr_core_watch_write( avr, addr, v );
    avr_regbit_setto( avr, p->twsr, sr );    // force restore

    if( c != avr_regbit_get( avr, p->twps) ) { // prescaler bits changed...
    }
}

static void avr_twi_irq_input( struct avr_irq_t* irq, uint32_t value, void* param )
{
    avr_twi_t* p = (avr_twi_t*)param;
    avr_t* avr = p->io.avr;

    if( !avr_regbit_get(avr, p->twen) ) return; // check to see if we are enabled

    avr_twi_msg_irq_t msg;
    msg.u.v = value;

    if( msg.u.twi.msg & TWI_COND_START ) // receiving an attempt at waking a slave
    {
        p->state = 0;
        p->peer_addr = 0;
        if( msg.u.twi.msg & TWI_COND_ADDR )
        {
            uint8_t mask = ~avr->data[p->r_twamr] >> 1;

            p->peer_addr = msg.u.twi.addr & mask;
            if( p->peer_addr == ((avr->data[p->r_twar] >> 1) & mask) ) // address match, we're talking
            {
                p->state = TWI_COND_SLAVE;

                if( !(msg.u.twi.msg & TWI_COND_WRITE) ) p->peer_addr |= 1; // INVERSE logic here

                uint8_t sta = msg.u.twi.msg & TWI_COND_WRITE ? TWI_SRX_ADR_ACK : TWI_STX_ADR_ACK;
                _avr_twi_status_set( p, sta, 1);
            }
        }
        else
        {
            if( p->state & TWI_COND_START )
                _avr_twi_status_set( p, TWI_REP_START, 1);
            else
            {
                _avr_twi_status_set( p, TWI_START, 1);
                p->peer_addr = 0;
                p->state = TWI_COND_START;
            }
        }
        /*else // "general call" address
        {
            if( avr->data[p->r_twar] & 1 )  { // TODO
            }
        }*/
    }
    if( msg.u.twi.msg & TWI_COND_STOP ) // clear stop condition regardless of status
    {
        avr_regbit_clear(avr, p->twsto);
        _avr_twi_status_set( p, TWI_NO_STATE, 0 );
        p->state = 0;
    }

    if( msg.u.twi.msg & TWI_COND_ACK )// receiving an acknowledge bit
    {
        if( msg.u.twi.data & 1 ) p->state |= TWI_COND_ACK;
        else                     p->state &= ~TWI_COND_ACK;

        int do_read = p->peer_addr & 1;
        int ack = p->state & TWI_COND_ACK;

        uint8_t sta = 0;

        if( p->state & TWI_COND_ADDR ) // writing or reading a byte
        {
            if( do_read )
                sta = ack ? TWI_MRX_DATA_ACK : TWI_MRX_DATA_NACK;
            else
                sta = ack ? TWI_MTX_DATA_ACK : TWI_MTX_DATA_NACK;

        }
        else if( p->state ) // send the address
        {
            p->state |= TWI_COND_ADDR;

            if( do_read )
                sta = ack ? TWI_MRX_ADR_ACK : TWI_MRX_ADR_NACK;
            else
                sta = ack ? TWI_MTX_ADR_ACK : TWI_MTX_ADR_NACK;
        }

        _avr_twi_status_set( p, sta, 1);
    }
    if( p->state & TWI_COND_SLAVE )
    {
        if( msg.u.twi.msg & TWI_COND_WRITE )
        {
            avr->data[p->r_twdr] = msg.u.twi.data;
            _avr_twi_status_set( p, TWI_SRX_ADR_DATA_ACK, 1);
        }
    }
    else // receive a data byte from a slave
    {
        if( msg.u.twi.msg & TWI_COND_READ ) avr->data[p->r_twdr] = msg.u.twi.data;
    }
}

void avr_twi_reset( struct avr_io_t* io )
{
    avr_twi_t* p = (avr_twi_t*)io;
    avr_irq_register_notify(p->io.irq + TWI_IRQ_INPUT, avr_twi_irq_input, p);
    p->state = p->peer_addr = 0;
    avr_regbit_setto_raw(p->io.avr, p->twsr, TWI_NO_STATE);
}

static const char* irq_names[TWI_IRQ_COUNT] = {
    [TWI_IRQ_INPUT]  = "8<input",
    [TWI_IRQ_OUTPUT] = "32>output",
    [TWI_IRQ_STATUS] = "8>status",
};

static    avr_io_t    _io = {
    .kind = "twi",
    .reset = avr_twi_reset,
    .irq_names = irq_names,
};

void avr_twi_init( avr_t* avr, avr_twi_t* p )
{
    p->io = _io;
    avr_register_io( avr, &p->io );
    avr_register_vector( avr, &p->twi );

    // allocate this module's IRQ
    avr_io_setirqs( &p->io, AVR_IOCTL_TWI_GETIRQ(p->name), TWI_IRQ_COUNT, NULL );

    avr_register_io_write( avr, p->twen.reg, avr_twi_write,      p);
    avr_register_io_write( avr, p->r_twdr,   avr_twi_write_data, p);
    avr_register_io_read(  avr, p->r_twdr,   avr_twi_read_data,  p);
    avr_register_io_write( avr, p->twsr.reg, avr_twi_write_status, p);
}

uint32_t avr_twi_irq_msg( uint8_t msg, uint8_t addr, uint8_t data)
{
    avr_twi_msg_irq_t _msg = {
            .u.twi.msg  = msg,
            .u.twi.addr = addr,
            .u.twi.data = data,
    };
    return _msg.u.v;
}
