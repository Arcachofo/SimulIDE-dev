/*
   Copyright (C) 1998 T. Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see 
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/

// p16x6x
//
//  This file supports:
//    P16C61

#include <stdio.h>
#include <iostream>
#include <string>

#include "p16x6x.h"
#include "pic-ioports.h"
#include "intcon.h"


//---------------------------------------------------------
//
//  P16x6x::create_sfr_map(void) - Here's where all of the
//  registers are defined for a p16c63 and greater...
void P16X6X_processor::create_sfr_map()
{
    Pic14Bit::create_sfr_map();

    // P16x63 and higher have porta5
    m_porta->setEnableMask(0x3f);
    m_porta->setTris(m_trisa);

    // The 16c62,c64 have general purpose registers
    // at addresses 20-7f and a0-bf
    add_file_registers(0x20, 0x7f, 0);
    add_file_registers(0xa0, 0xbf, 0);


    add_SfrReg(pir1,   0x0c, RegisterValue(0,0),"pir1");
    add_SfrReg(&pie1,   0x8c, RegisterValue(0,0));

    add_SfrReg(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
    add_SfrReg(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

    add_SfrReg(&pcon,   0x8e, RegisterValue(0,0),"pcon");

    add_SfrReg(&t1con,  0x10, RegisterValue(0,0));
    add_SfrReg(&tmr2,   0x11, RegisterValue(0,0));
    add_SfrReg(&t2con,  0x12, RegisterValue(0,0));
    add_SfrReg(&pr2,    0x92, RegisterValue(0xff,0));

    if( hasSSP() ) {
        add_SfrReg(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
        add_SfrReg(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
        add_SfrReg(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
        add_SfrReg(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
        tmr2.ssp_module[0] = &ssp;
    }

    add_SfrReg(&ccpr1l,  0x15, RegisterValue(0,0));
    add_SfrReg(&ccpr1h,  0x16, RegisterValue(0,0));
    add_SfrReg(&ccp1con, 0x17, RegisterValue(0,0));

    // get_pir_set()->set_pir1(get_pir1());
    pir_set_def.set_pir1(pir1);

    intcon = &intcon_reg;
    intcon_reg.set_pir_set(get_pir_set());

    // Maybe there's a better place for this, but let's go ahead and link all
    // of the registers together (there's probably a better way too) :

    tmr1l.tmrh = &tmr1h;
    tmr1l.t1con = &t1con;
    // FIXME -- can't delete this new'd item
    tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));
    //  tmr1l.ccpcon = &ccp1con;

    tmr1h.tmrl  = &tmr1l;
    t1con.tmrl  = &tmr1l;

    t2con.tmr2  = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
    tmr2.add_ccp ( &ccp2con );
    pr2.tmr2    = &tmr2;

    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v1::CCP1IF, &tmr2);
    ccp1con.setIOpin(&((*m_portc)[2]));
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;

    //  portc->ccp1con = &ccp1con;

    ccpr1l.new_name("ccpr1l");
    ccpr1h.new_name("ccpr1h");
    ccp1con.new_name("ccp1con");

    if (pir1) {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
}

//--------------------------------------------------
P16X6X_processor::P16X6X_processor(const char *_name)
    : Pic14Bit(_name ),
      t1con(this, "t1con" ),
      pie1(this,"PIE1" ),
      pie2(this,"PIE2" ),
      t2con(this, "t2con" ),
      pr2(this, "pr2" ),
      tmr2(this, "tmr2" ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      ccp1con(this, "ccp1con" ),
      ccpr1l(this, "ccpr1l" ),
      ccpr1h(this, "ccpr1h" ),
      ccp2con(this, "ccp2con" ),
      ccpr2l(this, "ccpr2l" ),
      ccpr2h(this, "ccpr2h" ),
      pcon(this, "pcon" ),
      ssp(this)
{
    m_portc = new PicPortRegister(this,"portc", 8,0xff);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);

    pir1 = new PIR1v1(this,"pir1", &intcon_reg, &pie1);
    pir2 = new PIR2v1(this,"pir2", &intcon_reg, &pie2);
}

P16X6X_processor::~P16X6X_processor()
{
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&pcon);
    remove_SfrReg(&t1con);
    remove_SfrReg(&tmr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&pr2);

    if( hasSSP()) {
        remove_SfrReg(&ssp.sspbuf);
        remove_SfrReg(&ssp.sspcon);
        remove_SfrReg(&ssp.sspadd);
        remove_SfrReg(&ssp.sspstat);
    }
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&pie1);

    delete_file_registers(0x20,0x7f);
    delete_file_registers(0xa0,0xbf);
    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);
    delete_SfrReg(pir2);
    delete_SfrReg(pir1);
}

/*******************************************************************
 *
 *        Definitions for the various P16x6x processors
 *
 */

void P16C61::create(void)
{
    create_iopin_map();

    _14bit_processor::create();

    add_file_registers(0x0c, 0x2f, 0x80);
    Pic14Bit::create_sfr_map();
}

Processor * P16C61::construct(const char *name)
{
    P16C61 *p = new P16C61(name);

    p->create();
    p->create_invalid_registers ();

    return p;
}

P16C61::P16C61(const char *_name)
    : P16X8X(_name )
{
    ram_top = 0x2f;
}
P16C61::~P16C61()
{
}

//------------------------------------------------------------------------
//
void P16C62::create_iopin_map(void)
{
    assign_pin(1, 0);

    assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
    assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
    assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
    assign_pin( 6, m_porta->addPin( new IOPIN("porta4", OPEN_COLLECTOR),4) );
    assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

    assign_pin(8, 0); //VSS
    assign_pin(9, 0);  // OSC
    assign_pin(10, 0); // OSC

    assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
    assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
    assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
    assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));
    assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
    assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
    assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
    assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));

    assign_pin(19, 0); //VSS
    assign_pin(20, 0); //VDD
    assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
    assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
    assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
    assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
    assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
    assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
    assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
    assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

    if (hasSSP()) {
        ssp.initialize(
                    get_pir_set(),    // PIR
                    &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // I2C port
                SSP_TYPE_BSSP
                );
    }

    tmr1l.setIOpin(&(*m_portc)[0]);
}

P16C62::P16C62(const char *_name )
    : P16X6X_processor(_name )
{
    set_hasSSP();
}
P16C62::~P16C62()
{
}

void P16C62::create_sfr_map()
{
    P16X6X_processor::create_sfr_map();

    add_SfrReg(m_portc, 0x07);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));

    //1((PORTC*)portc)->ccp1con = &ccp1con;
}

void  P16C62::create(void)
{
    create_iopin_map();

    _14bit_processor::create();

    P16C62::create_sfr_map();

    // Build the links between the I/O Pins and the internal peripherals
    //1ccp1con.iopin = portc->pins[2];
}

Processor * P16C62::construct(const char *name)
{
    P16C62 *p = new P16C62(name);

    p->create();
    p->create_invalid_registers ();

    return p;
}

//------------------------------------------------------------------------
//
void P16C63::create_sfr_map(void)
{
    add_file_registers(0xc0, 0xff, 0);

    add_SfrReg(pir2,    0x0d, RegisterValue(0,0),"pir2");
    add_SfrReg(&pie2,   0x8d, RegisterValue(0,0));

    add_SfrReg(&ccpr2l, 0x1b, RegisterValue(0,0));
    add_SfrReg(&ccpr2h, 0x1c, RegisterValue(0,0));
    add_SfrReg(&ccp2con, 0x1d, RegisterValue(0,0));

    // get_pir_set()->set_pir2(get_pir2());
    pir_set_def.set_pir2(pir2);

    ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v1::CCP2IF, &tmr2);
    ccp2con.setIOpin(&((*m_portc)[1]));
    ccpr2l.ccprh  = &ccpr2h;
    ccpr2l.tmrl   = &tmr1l;
    ccpr2h.ccprl  = &ccpr2l;

    usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
            new _TXREG(this,"txreg", &usart),
            new _RCREG(this,"rcreg", &usart));

    add_SfrReg(&usart.rcsta, 0x18, RegisterValue(0,0),0);
    add_SfrReg(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
    add_SfrReg(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");

    if (pir2)
    {
        pir2->set_intcon(&intcon_reg);
        pir2->set_pie(&pie2);
    }
    pie2.setPir(get_pir2());
}

//------------------------------------------------------------------------
//
// P16C63 constructor
//
// Note: Since the 'C63 is derived from the 'C62. So before this constructor
// is called, the C62 constructor will be called. Most of the initialization
// is done within the 'C62 constructor.

P16C63::P16C63( const char *_name )
    : P16C62(_name ),
      usart(this)
{
}

P16C63::~P16C63()
{
    remove_SfrReg(&pie2);
    remove_SfrReg(&ccpr2l);
    remove_SfrReg(&ccpr2h);
    remove_SfrReg(&ccp2con);
    if (registers[0xf0]->alias_mask & 0x80)
        delete_file_registers(0xc0, 0xef);
    else
        delete_file_registers(0xc0, 0xff);
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);

    //delete_SfrReg(pir2,0x0d);
}

void P16C63::create(void)
{
    P16C62::create();

    P16C63::create_sfr_map();

    // Build the links between the I/O Pins and the internal peripherals
    //1ccp2con.iopin = portc->pins[1];
}

Processor * P16C63::construct(const char *name)
{
    P16C63 *p = new P16C63(name);

    p->create();
    p->create_invalid_registers ();

    return p;
}

//----------------------------------------------------------
//
P16C64::P16C64(const char *_name)
    : P16X6X_processor(_name )
{
    set_hasSSP();
    pir1_2_reg = new PIR1v2(this,"pir1", &intcon_reg,&pie1);
    delete pir1;
    pir1 = pir1_2_reg;


    m_portd = new PicPSP_PortRegister(this,"portd", 8,0xff);
    m_trisd = new PicTrisRegister(this,"trisd", (PicPortRegister *)m_portd, false);

    m_porte = new PicPortRegister(this,"porte", 8,0x07);
    m_trise =  new PicPSP_TrisRegister(this,"trise", m_porte, false);
}
P16C64::~P16C64()
{
    delete_SfrReg(m_portd);
    delete_SfrReg(m_trisd);

    delete_SfrReg(m_porte);
    delete_SfrReg(m_trise);
}

void P16C64::create_iopin_map(void)
{
    assign_pin(1, 0);

    assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
    assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
    assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
    assign_pin( 6, m_porta->addPin( new IOPIN("porta4", OPEN_COLLECTOR),4) );
    assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

    assign_pin( 8, m_porte->addPin(new IO_bi_directional("porte0"),0));
    assign_pin( 9, m_porte->addPin(new IO_bi_directional("porte1"),1));
    assign_pin(10, m_porte->addPin(new IO_bi_directional("porte2"),2));

    assign_pin(11, 0);
    assign_pin(12, 0);
    assign_pin(13, 0);
    assign_pin(14, 0);

    assign_pin(15, m_portc->addPin(new IO_bi_directional("portc0"),0));
    assign_pin(16, m_portc->addPin(new IO_bi_directional("portc1"),1));
    assign_pin(17, m_portc->addPin(new IO_bi_directional("portc2"),2));
    assign_pin(18, m_portc->addPin(new IO_bi_directional("portc3"),3));
    assign_pin(23, m_portc->addPin(new IO_bi_directional("portc4"),4));
    assign_pin(24, m_portc->addPin(new IO_bi_directional("portc5"),5));
    assign_pin(25, m_portc->addPin(new IO_bi_directional("portc6"),6));
    assign_pin(26, m_portc->addPin(new IO_bi_directional("portc7"),7));


    assign_pin(19, m_portd->addPin(new IO_bi_directional("portd0"),0));
    assign_pin(20, m_portd->addPin(new IO_bi_directional("portd1"),1));
    assign_pin(21, m_portd->addPin(new IO_bi_directional("portd2"),2));
    assign_pin(22, m_portd->addPin(new IO_bi_directional("portd3"),3));
    assign_pin(27, m_portd->addPin(new IO_bi_directional("portd4"),4));
    assign_pin(28, m_portd->addPin(new IO_bi_directional("portd5"),5));
    assign_pin(29, m_portd->addPin(new IO_bi_directional("portd6"),6));
    assign_pin(30, m_portd->addPin(new IO_bi_directional("portd7"),7));

    assign_pin(31, 0);
    assign_pin(32, 0);

    assign_pin(33, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
    assign_pin(34, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
    assign_pin(35, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
    assign_pin(36, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
    assign_pin(37, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
    assign_pin(38, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
    assign_pin(39, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
    assign_pin(40, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

    if (hasSSP()) {
        ssp.initialize(
                    get_pir_set(),    // PIR
                    &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // I2C port
                SSP_TYPE_BSSP
                );
    }
    psp.initialize(get_pir_set(),    // PIR
                   m_portd,           // Parallel port
                   m_trisd,           // Parallel tris
                   m_trise,           // Control tris
                   &(*m_porte)[0],    // NOT RD
            &(*m_porte)[1],    // NOT WR
            &(*m_porte)[2]);   // NOT CS

    tmr1l.setIOpin(&(*m_portc)[0]);
}

void P16C64::create_sfr_map(void)
{
    pir_set_2_def.set_pir1(pir1_2_reg);

    P16X6X_processor::create_sfr_map();

    add_SfrReg(m_portc, 0x07);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));

    add_SfrReg(m_portd, 0x08);
    add_SfrReg(m_trisd, 0x88, RegisterValue(0xff,0));

    add_SfrReg(m_porte, 0x09);
    add_SfrReg(m_trise, 0x89, RegisterValue(0x07,0));

    //1((PORTC*)portc)->ccp1con = &ccp1con;
}

void  P16C64::create(void)
{
    create_iopin_map();

    _14bit_processor::create();

    //P16X6X_processor::create_sfr_map();
    P16C64::create_sfr_map();

    // Build the links between the I/O Pins and the internal peripherals
    //1ccp1con.iopin = portc->pins[2];
}

Processor * P16C64::construct(const char *name)
{
    P16C64 *p = new P16C64(name);

    p->create();
    p->create_invalid_registers ();

    return p;
}


//------------------------------------------------------------------------
//
//

void P16C65::create_sfr_map(void)
{
    //P16C64::create_sfr_map();

    add_file_registers(0xc0, 0xff, 0);

    add_SfrReg(pir2,    0x0d, RegisterValue(0,0),"pir2");
    add_SfrReg(&pie2,   0x8d, RegisterValue(0,0));

    add_SfrReg(&ccpr2l, 0x1b, RegisterValue(0,0));
    add_SfrReg(&ccpr2h, 0x1c, RegisterValue(0,0));
    add_SfrReg(&ccp2con, 0x1d, RegisterValue(0,0));

    // get_pir_set()->set_pir2(&get_pir2());
    pir_set_def.set_pir2(pir2);

    ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v2::CCP2IF, &tmr2);
    ccp2con.setIOpin(&((*m_portc)[1]));

    ccpr2l.ccprh  = &ccpr2h;
    ccpr2l.tmrl   = &tmr1l;
    ccpr2h.ccprl  = &ccpr2l;

    usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
            new _TXREG(this,"txreg", &usart),
            new _RCREG(this,"rcreg", &usart));

    add_SfrReg(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
    add_SfrReg(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
    add_SfrReg(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");

    if (pir2)
    {
        pir2->set_intcon(&intcon_reg);
        pir2->set_pie(&pie2);
    }

    pie2.setPir(get_pir2());
}

//------------------------------------------------------------------------
//
// P16C65 constructor
//
// Note: Since the 'C65 is derived from the 'C64. So before this constructor
// is called, the C64 constructor will be called. Most of the initialization
// is done within the 'C64 constructor.

P16C65::P16C65(const char *_name )
    : P16C64(_name ),
      usart(this)
{
}
P16C65::~P16C65()
{
    if (registers[0xf0]->alias_mask & 0x80)
        delete_file_registers(0xc0, 0xef);
    else
        delete_file_registers(0xc0, 0xff);
    remove_SfrReg(&ccpr2l);
    remove_SfrReg(&ccpr2h);
    remove_SfrReg(&ccp2con);
    remove_SfrReg(&pie2);
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);
}

void P16C65::create(void)
{
    P16C64::create();

    P16C65::create_sfr_map();

    // Build the links between the I/O Pins and the internal peripherals
    // ccp1con.iopin = portc.pins[2];
    //1ccp2con.iopin = portc->pins[1];
}

Processor * P16C65::construct(const char *name)
{
    P16C65 *p = new P16C65(name);

    p->create();
    p->create_invalid_registers ();

    return p;
}

//========================================================================
//
// Configuration Memory for 16F630/676

class ConfigF630 : public ConfigWord
{
public:
    ConfigF630(P16F630 *pCpu)
        : ConfigWord("CONFIG", 0x3fff, pCpu, 0x2007)
    {
        if (m_pCpu) m_pCpu->set_config_word(0x2007, 0x3fff);
    }

    enum {
        FOSC0  = 1<<0,
        FOSC1  = 1<<1,
        FOSC2  = 1<<2,
        WDTEN  = 1<<3,
        PWRTEN = 1<<4,
        MCLRE =  1<<5,
        BODEN =  1<<6,
        CP =     1<<7,
        CPD =    1<<8
    };

    string toString()
    {
        int64_t i64;
        get(i64);
        int i = i64 &0xfff;

        char buff[356];

        const char *OSCdesc[8] =
        {
            "LP oscillator",
            "XT oscillator",
            "HS oscillator",
            "EC oscillator w/ OSC2 configured as I/O",
            "INTOSC oscillator: I/O on RA4 pin, I/O on RA5",
            "INTOSC oscillator: CLKOUT on RA4 pin, I/O on RA5",
            "RC oscillator: I/O on RA4 pin, RC on RA5",
            "RC oscillator: CLKOUT on RA4 pin, RC on RA5"
        };
        snprintf(buff,sizeof(buff),
                 " $%04x\n"
                 " FOSC=%d - Clk source = %s\n"
                 " WDTEN=%d - WDT is %s\n"
                 " PWRTEN=%d - Power up timer is %s\n"
                 " MCLRE=%d - RA3 Pin %s\n"
                 " BODEN=%d -  Brown-out Detect %s\n"
                 " CP=%d - Code Protection %s\n"
                 " CPD=%d -  Data Code Protection %s\n",
                 i,
                 i&(FOSC0|FOSC1|FOSC2), OSCdesc[i&(FOSC0|FOSC1|FOSC2)],
                ((i&WDTE) ? 1 : 0), ((i&WDTE) ? "enabled" : "disabled"),
                ((i&PWRTEN) ? 1 : 0), ((i&PWRTEN) ? "disabled" : "enabled"),
                ((i&MCLRE) ? 1 : 0), ((i&MCLRE) ? "MCLR" : "Input"),
                ((i&BODEN) ? 1 : 0), ((i&BODEN) ? "enabled" : "disabled"),
                ((i&CP) ? 1 : 0), ((i&CP) ? "disabled" : "enabled"),
                ((i&CPD) ? 1 : 0), ((i&CPD) ? "disabled" : "enabled")
                );
        return string(buff);
    }
};

//------------------------------------------------------------------------
//
P16F630::P16F630(const char *_name )
    : _14bit_processor(_name ),
      t1con(this, "t1con" ),
      pie1(this,"PIE1" ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      osccal(this, "osccal", 0xfc),
      intcon_reg(this,"intcon" ),
      comparator(this)
{
    pir1_3_reg = new PIR1v3(this,"pir1", &intcon_reg,&pie1);
    pir1 = pir1_3_reg;

    m_ioc = new IOC(this, "ioc" );

    m_porta = new PicPortGRegister(this,"porta", &intcon_reg, m_ioc, 8,0x3f);
    m_trisa = new PicTrisRegister(this,"trisa", m_porta, false);

    m_wpu = new WPU(this, "wpu", m_porta, 0x37);
    tmr0.set_cpu(this, m_porta, 4, option_reg);
    tmr0.start(0);

    m_portc = new PicPortRegister(this,"portc", 8,0x3f);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);
}

P16F630::~P16F630()
{
    unassignMCLRPin();
    delete_file_registers(0x20, 0x5f);
    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);

    delete_SfrReg(m_porta);
    delete_SfrReg(m_trisa);
    delete_SfrReg(m_ioc);
    delete_SfrReg(m_wpu);
    delete_SfrReg(pir1_3_reg);
    remove_SfrReg(&tmr0);
    remove_SfrReg(&intcon_reg);
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&t1con);
    remove_SfrReg(&comparator.cmcon);
    remove_SfrReg(&comparator.vrcon);
    remove_SfrReg(&osccal);
    remove_SfrReg(&pie1);
    delete e;
}

void P16F630::create_iopin_map(void)
{
    assign_pin(1, 0);        // Vdd

    assign_pin( 2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
    assign_pin( 4, m_porta->addPin(new IOPIN("porta3"),3));
    assign_pin( 5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
    assign_pin( 6, m_portc->addPin(new IO_bi_directional("portc4"),4));
    assign_pin( 7, m_portc->addPin(new IO_bi_directional("portc3"),3));

    assign_pin( 8, m_portc->addPin(new IO_bi_directional("portc2"),2));
    assign_pin( 9, m_portc->addPin(new IO_bi_directional("portc1"),1));
    assign_pin(10, m_portc->addPin(new IO_bi_directional("portc0"),0));

    assign_pin(11, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
    assign_pin(12, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
    assign_pin(13, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));

    assign_pin(14, 0); //VSS

    tmr1l.setIOpin(&(*m_portc)[0]);
}

Processor * P16F630::construct(const char *name)
{
    P16F630 *p = new P16F630(name);

    p->create(128);
    p->create_invalid_registers ();

    return p;
}

void P16F630::create(int eesize)
{
    create_iopin_map();

    _14bit_processor::create();

    e = new EEPROM_WIDE(this,pir1);
    e->initialize(eesize);
    e->set_intcon(&intcon_reg);
    set_eeprom_wide(e);

    P16F630::create_sfr_map();
}

void P16F630::create_sfr_map()
{
    pir_set_def.set_pir1(pir1);

    add_file_registers(0x20, 0x5f, 0);
    alias_file_registers(0x20, 0x5f, 0x80);

    add_SfrReg(indf,    0x00);
    alias_file_registers(0x00,0x00,0x80);

    add_SfrReg(&tmr0,   0x01);
    add_SfrReg(option_reg,  0x81, RegisterValue(0xff,0));

    add_SfrReg(pcl,     0x02, RegisterValue(0,0));
    add_SfrReg(status,  0x03, RegisterValue(0x18,0));
    add_SfrReg(fsr,     0x04);
    alias_file_registers(0x02,0x04,0x80);

    add_SfrReg(m_porta, 0x05);
    add_SfrReg(m_trisa, 0x85, RegisterValue(0x3f,0));

    add_SfrReg(m_portc, 0x07);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));

    add_SfrReg(pclath,  0x0a, RegisterValue(0,0));

    add_SfrReg(&intcon_reg, 0x0b, RegisterValue(0,0));
    alias_file_registers(0x0a,0x0b,0x80);
    add_SfrReg(pir1, 0x0c, RegisterValue(0,0));
    add_SfrReg(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
    add_SfrReg(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
    add_SfrReg(&t1con, 0x10, RegisterValue(0,0));

    intcon = &intcon_reg;
    intcon_reg.set_pir_set(get_pir_set());

    tmr1l.tmrh = &tmr1h;
    tmr1l.t1con = &t1con;
    // FIXME -- can't delete this new'd item
    tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v3::TMR1IF));
    tmr1h.tmrl  = &tmr1l;
    t1con.tmrl  = &tmr1l;

    tmr1l.setIOpin(&(*m_porta)[5]);
    tmr1l.setGatepin(&(*m_porta)[4]);

    add_SfrReg(&pie1,   0x8c, RegisterValue(0,0));
    if (pir1) {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);


    // Link the comparator and voltage ref to porta
    comparator.initialize(get_pir_set(), NULL,
                          &(*m_porta)[0], &(*m_porta)[1],
            NULL, NULL,
            &(*m_porta)[2], NULL);

    comparator.cmcon.set_configuration(1, 0, AN0, AN1, AN0, AN1, ZERO);
    comparator.cmcon.set_configuration(1, 1, AN0, AN1, AN0, AN1, OUT0);
    comparator.cmcon.set_configuration(1, 2, AN0, AN1, AN0, AN1, NO_OUT);
    comparator.cmcon.set_configuration(1, 3, AN1, VREF, AN1, VREF, OUT0);
    comparator.cmcon.set_configuration(1, 4, AN1, VREF, AN1, VREF, NO_OUT);
    comparator.cmcon.set_configuration(1, 5, AN1, VREF, AN0, VREF, OUT0);
    comparator.cmcon.set_configuration(1, 6, AN1, VREF, AN0, VREF, NO_OUT);
    comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 0, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 1, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 2, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 3, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 4, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 6, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

    add_SfrReg(&comparator.cmcon, 0x19, RegisterValue(0,0),"cmcon");
    add_SfrReg(&comparator.vrcon, 0x99, RegisterValue(0,0),"cvrcon");

    add_SfrReg(get_eeprom()->get_reg_eedata(),  0x9a);
    add_SfrReg(get_eeprom()->get_reg_eeadr(),   0x9b);
    add_SfrReg(get_eeprom()->get_reg_eecon1(),  0x9c, RegisterValue(0,0));
    add_SfrReg(get_eeprom()->get_reg_eecon2(),  0x9d);
    add_SfrReg(m_wpu, 0x95, RegisterValue(0x37,0),"wpua");
    add_SfrReg(m_ioc, 0x96, RegisterValue(0,0),"ioca");
    add_SfrReg(&osccal, 0x90, RegisterValue(0x80,0));

}
//-------------------------------------------------------------------
void P16F630::option_new_bits_6_7(uint bits)
{
    m_wpu->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
    m_porta->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}
//-------------------------------------------------------------------
void P16F630::create_config_memory()
{
    m_configMemory = new ConfigMemory(this,1);
    m_configMemory->addConfigWord(0,new ConfigF630(this));

};

//-------------------------------------------------------------------
bool P16F630::set_config_word(uint address, uint cfg_word)
{
    enum {
        CFG_FOSC0 = 1<<0,
        CFG_FOSC1 = 1<<1,
        CFG_FOSC2 = 1<<2,
        CFG_WDTE  = 1<<3,
        CFG_MCLRE = 1<<5,
    };


    if(address == config_word_address())
    {
        uint valid_pins = m_porta->getEnableMask();

        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE)
        {
            assignMCLRPin(4);
        }
        else unassignMCLRPin();

        wdt.initialize((cfg_word & CFG_WDTE) == CFG_WDTE);

        set_int_osc(false);

        // AnalogReq is used so ADC does not change clock names
        // set_config_word is first called with default and then
        // often called a second time. the following call is to
        // reset porta so next call to AnalogReq sill set the pin name
        //
        (&(*m_porta)[4])->AnalogReq((Register *)this, false, "porta4");
        valid_pins |= 0x20;
        switch(cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2))
        {

        case 0:  // LP oscillator: low power crystal is on RA4 and RA5
        case 1:     // XT oscillator: crystal/resonator is on RA4 and RA5
        case 2:     // HS oscillator: crystal/resonator is on RA4 and RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "OSC2");
            valid_pins &= 0xcf;
            break;

        case 3:        // EC I/O on RA4 pin, CLKIN on RA5
            valid_pins &= 0xef;
            break;
            
        case 5: // INTOSC CLKOUT on RA4 pin
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");
        case 4: // INTOSC
            set_int_osc(true);
            osccal.set_freq(4e6);
            break;

        case 6: //RC oscillator: I/O on RA4 pin, RC on RA5
            valid_pins &= 0xdf;
            break;

        case 7: // RC oscillator: CLKOUT on RA4 pin, RC on RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");
            valid_pins &= 0xdf;
            break;
        };

        if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
        {
            m_porta->setEnableMask(valid_pins);
            m_trisa->setEnableMask(valid_pins);
        }
        return(true);
    }
    return false;
}

//------------------------------------------------------------------------
//
P16F676::P16F676(const char *_name )
    : P16F630(_name ),
      ansel(this,"ansel" ),
      adcon0(this,"adcon0" ),
      adcon1(this,"adcon1" ),
      adresh(this,"adresh" ),
      adresl(this,"adresl" )
{
}
P16F676::~P16F676()
{
    remove_SfrReg(&adresl);
    remove_SfrReg(&adresh);
    remove_SfrReg(&adcon0);
    remove_SfrReg(&adcon1);
    remove_SfrReg(&ansel);
}

Processor * P16F676::construct(const char *name)
{
    P16F676 *p = new P16F676(name);

    p->create(128);
    p->create_invalid_registers ();

    return p;
}

void P16F676::create(int ram_top)
{
    P16F630::create(ram_top);
    create_sfr_map();
}

void P16F676::create_sfr_map()
{
    add_SfrReg(&adresl,  0x9e, RegisterValue(0,0));
    add_SfrReg(&adresh,  0x1e, RegisterValue(0,0));

    add_SfrReg(&adcon0, 0x1f, RegisterValue(0,0));
    add_SfrReg(&adcon1, 0x9f, RegisterValue(0,0));
    add_SfrReg(&ansel, 0x91, RegisterValue(0xff,0));


    ansel.setAdcon1(&adcon1);
    //  ansel.setAdcon0(&adcon0);
    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(&intcon_reg);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(7);
    adcon0.setChannel_shift(2);

    adcon1.setAdcon0(&adcon0);
    adcon1.setNumberOfChannels(8);

    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);
    adcon1.setIOPin(4, &(*m_portc)[0]);
    adcon1.setIOPin(5, &(*m_portc)[1]);
    adcon1.setIOPin(6, &(*m_portc)[2]);
    adcon1.setIOPin(7, &(*m_portc)[3]);

    adcon1.setVrefHiConfiguration(2, 1);

    /* Channel Configuration done dynamiclly based on ansel */

}
