/*
   Copyright (C) 2013,2014,2017 Roy R. Rankin

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
/****************************************************************
*                                                               *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com    *
*                                                               *
*****************************************************************/

//
// p16f178x
//
//  This file supports:
//    PIC16[L]F1788
//
//Note: All these  processors have extended 14bit instructions

#include <stdio.h>
#include <iostream>
#include <string>

#include "eeprom.h"
#include "p16f178x.h"
#include "pic-ioports.h"
#include "apfcon.h"
#include "pir.h"

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


P16F178x::P16F178x(const char *_name)
  : _14bit_e_processor(_name ),
    comparator(this),
    pie1(this,"pie1"),
    pie2(this,"pie2" ),
    pie3(this,"pie3" ),
    pie4(this,"pie4" ),
    t2con(this, "t2con"),
    pr2(this, "pr2" ),
    tmr2(this, "tmr2" ),
    t1con_g(this, "t1con" ),
    tmr1l(this, "tmr1l" ),
    tmr1h(this, "tmr1h" ),
    ccp1con(this, "ccp1con"),
    ccpr1l(this, "ccpr1l" ),
    ccpr1h(this, "ccpr1h" ),
    fvrcon(this, "fvrcon", 0xbf, 0x40),
    borcon(this, "borcon" ),
    ansela(this, "ansela" ),
    anselb(this, "anselb" ),
    anselc(this, "anselc" ),
    adcon0(this,"adcon0" ),
    adcon1(this,"adcon1" ),
    adcon2(this,"adcon2" ),
    adresh(this,"adresh" ),
    adresl(this,"adresl" ),
    osccon(0),
    osctune(this, "osctune" ),
    oscstat(this, "oscstat" ),
    wdtcon(this, "wdtcon" , 0x3f),
    usart(this),
    ssp(this),
    apfcon1(this, "apfcon1", 0xff),
    apfcon2(this, "apfcon2", 0x07),
    pwm1con(this, "pwm1con" ),
    ccp1as(this, "ccp1as" ),
    pstr1con(this, "pstr1con" ),
    vregcon(this, "vregcon" )
{
  m_iocbf = new IOCxF(this, "iocbf" );
  m_iocbp = new IOC(this, "iocbp" );
  m_iocbn = new IOC(this, "iocbn" );
  m_portb= new PicPortIOCRegister(this,"portb", intcon, m_iocbp, m_iocbn, m_iocbf, 8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb", m_portb, false, 0xff);
  m_latb  = new PicLatchRegister(this,"latb",m_portb, 0xff);
  m_wpub = new WPU(this, "wpub" , m_portb, 0xff);

  m_ioccf = new IOCxF(this, "ioccf" );
  m_ioccp = new IOC(this, "ioccp" );
  m_ioccn = new IOC(this, "ioccn" );
  m_portc= new PicPortIOCRegister(this,"portc", intcon, m_ioccp, m_ioccn, m_ioccf, 8,0xff);
  m_trisc = new PicTrisRegister(this,"trisc", m_portc, false, 0xff);
  m_latc  = new PicLatchRegister(this,"latc",m_portc, 0xff);
  m_wpuc = new WPU(this, "wpuc", m_portc, 0xff);

  m_iocaf = new IOCxF(this, "iocaf" );
  m_iocap = new IOC(this, "iocap" );
  m_iocan = new IOC(this, "iocan" );
  m_porta= new PicPortIOCRegister(this,"porta", intcon, m_iocap, m_iocan, m_iocaf, 8,0xff);
  m_trisa = new PicTrisRegister(this,"trisa", m_porta, false, 0xff);
  m_lata  = new PicLatchRegister(this,"lata",m_porta, 0xff);
  m_iocef = new IOCxF(this, "iocef", 0x08);
  m_iocep = new IOC(this, "iocep", 0x08);
  m_iocen = new IOC(this, "iocen", 0x08);
  m_porte= new PicPortIOCRegister(this,"porte", intcon, m_iocep, m_iocen, m_iocef, 8,0x08);
  m_trise = new PicTrisRegister(this,"trise", m_porte, false, 0x00);
  m_daccon0 = new DACCON0(this, "dac1con0", 0xbd, 256);
  m_daccon1 = new DACCON1(this, "dac1con1", 0xff, m_daccon0);
  m_dac2con0 = new DACCON0(this, "dac2con0", 0xb4, 32);
  m_dac2con1 = new DACCON1(this, "dac2con1", 0x1f, m_dac2con0);
  m_dac3con0 = new DACCON0(this, "dac3con0", 0xb4, 32);
  m_dac3con1 = new DACCON1(this, "dac3con1", 0x1f, m_dac3con0);
  m_dac4con0 = new DACCON0(this, "dac4con0", 0xb4, 32);
  m_dac4con1 = new DACCON1(this, "dac4con1", 0x1f, m_dac4con0);
  m_cpu_temp = 30.0;


  tmr0.set_cpu(this, m_porta, 4, &option_reg);
  tmr0.start(0);
  tmr0.set_t1gcon(&t1con_g.t1gcon);
  set_mclr_pin(1);

  ((INTCON_14_PIR *)intcon)->write_mask = 0xfe;

  m_wpua = new WPU(this, "wpua", m_porta, 0xff);
  m_wpue = new WPU(this, "wpue", m_porte, 0x08);

  pir1 = new PIR1v1822(this,"pir1",intcon, &pie1);
  pir2 = new PIR2v1822(this,"pir2",intcon, &pie2);
  pir3 = new PIR3v178x(this,"pir3" ,intcon, &pie3);
  pir4 = new PIR3v178x(this,"pir4" ,intcon, &pie3);
  pir2->valid_bits |= PIR2v1822::C2IF | PIR2v1822::CCP2IF | PIR2v1822::C3IF | PIR2v1822::C4IF;
  pir2->writable_bits |= PIR2v1822::C2IF | PIR2v1822::CCP2IF | PIR2v1822::C3IF | PIR2v1822::C4IF;
  pir4->valid_bits = pir4->writable_bits = 0xff;

  comparator.cmxcon0[0] = new CMxCON0(this, "cm1con0", 0, &comparator);
  comparator.cmxcon1[0] = new CMxCON1(this, "cm1con1", 0, &comparator);
  comparator.cmout = new CMOUT(this, "cmout");
  comparator.cmxcon0[1] = new CMxCON0(this, "cm2con0", 1, &comparator);
  comparator.cmxcon1[1] = new CMxCON1(this, "cm2con1", 1, &comparator);
  comparator.cmxcon0[2] = new CMxCON0(this, "cm3con0", 2, &comparator);
  comparator.cmxcon1[2] = new CMxCON1(this, "cm3con1", 2, &comparator);
}

P16F178x::~P16F178x()
{
    unassignMCLRPin();
  delete_file_registers(0x20, 0x7f);
  uint ram = ram_size - 96; // first 96 bytes already added
  uint add;
  for(add = 0x80; ram >= 80; add += 0x80)
  {
        ram -= 80;
        delete_file_registers(add + 0x20, add + 0x6f);
  }
  if (ram > 0) delete_file_registers(add + 0x20 , add + 0x20 + ram -1);

    delete_SfrReg(m_iocap);
    delete_SfrReg(m_iocan);
    delete_SfrReg(m_iocaf);
    delete_SfrReg(m_iocbp);
    delete_SfrReg(m_iocbn);
    delete_SfrReg(m_iocbf);
    delete_SfrReg(m_ioccp);
    delete_SfrReg(m_ioccn);
    delete_SfrReg(m_ioccf);
    delete_SfrReg(m_iocep);
    delete_SfrReg(m_iocen);
    delete_SfrReg(m_iocef);
    delete_SfrReg(m_daccon0);
    delete_SfrReg(m_daccon1);
    delete_SfrReg(m_dac2con0);
    delete_SfrReg(m_dac2con1);
    delete_SfrReg(m_dac3con0);
    delete_SfrReg(m_dac3con1);
    delete_SfrReg(m_dac4con0);
    delete_SfrReg(m_dac4con1);

    delete_SfrReg(m_trisa);
    delete_SfrReg(m_porta);
    delete_SfrReg(m_lata);
    delete_SfrReg(m_wpua);
    delete_SfrReg(m_portb);
    delete_SfrReg(m_trisb);
    delete_SfrReg(m_latb);
    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);
    delete_SfrReg(m_latc);
    delete_SfrReg(m_wpub);
    delete_SfrReg(m_wpuc);
    delete_SfrReg(m_trise);
    delete_SfrReg(m_porte);
    delete_SfrReg(m_wpue);

    remove_SfrReg(&tmr0);

    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&t1con_g);
    remove_SfrReg(&t1con_g.t1gcon);

    remove_SfrReg(&tmr2);
    remove_SfrReg(&pr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&ssp.sspbuf);
    remove_SfrReg(&ssp.sspadd);
    remove_SfrReg(ssp.sspmsk);
    remove_SfrReg(&ssp.sspstat);
    remove_SfrReg(&ssp.sspcon);
    remove_SfrReg(&ssp.sspcon2);
    remove_SfrReg(&ssp.ssp1con3);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&pwm1con);
    remove_SfrReg(&ccp1as);
    remove_SfrReg(&pstr1con);
    remove_SfrReg(&pie1);
    remove_SfrReg(&pie2);
    remove_SfrReg(&pie3);
    remove_SfrReg(&pie4);
    remove_SfrReg(&adresl);
    remove_SfrReg(&adresh);
    remove_SfrReg(&adcon0);
    remove_SfrReg(&adcon1);
    remove_SfrReg(&adcon2);
    remove_SfrReg(&borcon);
    remove_SfrReg(&fvrcon);
    remove_SfrReg(&apfcon1);
    remove_SfrReg(&apfcon2);
    remove_SfrReg(&ansela);
    remove_SfrReg(&anselb);
    remove_SfrReg(&anselc);
    remove_SfrReg(get_eeprom()->get_reg_eeadr());
    remove_SfrReg(get_eeprom()->get_reg_eeadrh());
    remove_SfrReg(get_eeprom()->get_reg_eedata());
    remove_SfrReg(get_eeprom()->get_reg_eedatah());
    remove_SfrReg(get_eeprom()->get_reg_eecon1());
    remove_SfrReg(get_eeprom()->get_reg_eecon2());
    remove_SfrReg(&usart.spbrg);
    remove_SfrReg(&usart.spbrgh);
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.baudcon);
    remove_SfrReg(&ssp.sspbuf);
    remove_SfrReg(&ssp.sspadd);
    remove_SfrReg(ssp.sspmsk);
    remove_SfrReg(&ssp.sspstat);
    remove_SfrReg(&ssp.sspcon);
    remove_SfrReg(&ssp.sspcon2);
    remove_SfrReg(&ssp.ssp1con3);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&pwm1con);
    remove_SfrReg(&ccp1as);
    remove_SfrReg(&pstr1con);
    remove_SfrReg(&osctune);
    remove_SfrReg(&option_reg);
    remove_SfrReg(osccon);
    remove_SfrReg(&oscstat);
    remove_SfrReg(&vregcon);

    remove_SfrReg(comparator.cmxcon0[0]);
    remove_SfrReg(comparator.cmxcon1[0]);
    remove_SfrReg(comparator.cmout);
    remove_SfrReg(comparator.cmxcon0[1]);
    remove_SfrReg(comparator.cmxcon1[1]);
    remove_SfrReg(comparator.cmxcon0[2]);
    remove_SfrReg(comparator.cmxcon1[2]);
    delete_SfrReg(usart.rcreg);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(pir1);
    delete_SfrReg(pir2);
    delete_SfrReg(pir3);
    delete_SfrReg(pir4);
    delete e;
}

void P16F178x::create_sfr_map()
{
  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);
  pir_set_2_def.set_pir3(pir3);
  pir_set_2_def.set_pir4(pir4);

  add_file_registers(0x20, 0x7f, 0x00);
  uint ram = ram_size - 96; // first 96 bytes already added
  uint add;
  for(add = 0x80; ram >= 80; add += 0x80)
  {
        ram -= 80;
        add_file_registers(add + 0x20, add + 0x6f, 0x00);
  }
  if (ram > 0) add_file_registers(add + 0x20 , add + 0x20 + ram -1, 0x00);

    add_SfrReg(m_porta, 0x0c);
    add_SfrReg(m_portb, 0x0d);
    add_SfrReg(m_portc, 0x0e);
    add_SfrReg(m_porte, 0x10);
    add_SfrRegR(pir1,    0x11, RegisterValue(0,0),"pir1");
    add_SfrRegR(pir2,    0x12, RegisterValue(0,0),"pir2");
    add_SfrRegR(pir3,    0x13, RegisterValue(0,0),"pir3");
    add_SfrRegR(pir4,    0x14, RegisterValue(0,0),"pir4");
    add_SfrReg(&tmr0,   0x15);

    add_SfrReg(&tmr1l,  0x16, RegisterValue(0,0),"tmr1l");
    add_SfrReg(&tmr1h,  0x17, RegisterValue(0,0),"tmr1h");
    add_SfrReg(&t1con_g,  0x18, RegisterValue(0,0));
    add_SfrReg(&t1con_g.t1gcon, 0x19, RegisterValue(0,0));

    add_SfrReg(&tmr2,   0x1a, RegisterValue(0,0));
    add_SfrReg(&pr2,    0x1b, RegisterValue(0,0));
    add_SfrRegR(&t2con,  0x1c, RegisterValue(0,0));


    add_SfrReg(m_trisa, 0x8c, RegisterValue(0xff,0));
    add_SfrReg(m_trisb, 0x8d, RegisterValue(0xff,0));
    add_SfrReg(m_trisc, 0x8e, RegisterValue(0xff,0));
    add_SfrReg(m_trise, 0x90, RegisterValue(0x08,0));

  pcon.valid_bits = 0xcf;
  add_SfrReg(&option_reg, 0x95, RegisterValue(0xff,0));
  add_SfrReg(&osctune,    0x98, RegisterValue(0,0));
  add_SfrReg(osccon,     0x99, RegisterValue(0x38,0));
  add_SfrReg(&oscstat,    0x9a, RegisterValue(0,0));

  intcon_reg.set_pir_set(get_pir_set());


  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con_g;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));

  tmr1h.tmrl  = &tmr1l;
  t1con_g.tmrl  = &tmr1l;
  t1con_g.t1gcon.set_tmrl(&tmr1l);
  t1con_g.t1gcon.setInterruptSource(new InterruptSource(pir1, PIR1v1822::TMR1IF));

  tmr1l.setIOpin(&(*m_porta)[5]);
  t1con_g.t1gcon.setGatepin(&(*m_porta)[3]);

  add_SfrRegR(&pie1,   0x91, RegisterValue(0,0));
  add_SfrRegR(&pie2,   0x92, RegisterValue(0,0));
  add_SfrRegR(&pie3,   0x93, RegisterValue(0,0));
  add_SfrRegR(&pie4,   0x94, RegisterValue(0,0));
  add_SfrReg(&adresl,  0x9b);
  add_SfrReg(&adresh,  0x9c);
  add_SfrRegR(&adcon0, 0x9d, RegisterValue(0x00,0));
  add_SfrRegR(&adcon1, 0x9e, RegisterValue(0x00,0));
  add_SfrRegR(&adcon2, 0x9f, RegisterValue(0x00,0));

  usart.initialize(pir1,
        &(*m_porta)[0], // TX pin
        &(*m_porta)[1], // RX pin
        new _TXREG( this, "txreg", &usart),
        new _RCREG( this, "rcreg", &usart));

  usart.set_eusart( true );

    add_SfrReg(m_lata, 0x10c);
    add_SfrReg(m_latb, 0x10d);
    add_SfrReg(m_latc, 0x10e);
    add_SfrRegR(comparator.cmxcon0[0], 0x111, RegisterValue(0x04,0));
    add_SfrRegR(comparator.cmxcon1[0], 0x112, RegisterValue(0x00,0));
    add_SfrRegR(comparator.cmxcon0[1], 0x113, RegisterValue(0x04,0));
    add_SfrRegR(comparator.cmxcon1[1], 0x114, RegisterValue(0x00,0));
    add_SfrRegR(comparator.cmout,      0x115, RegisterValue(0x00,0));
    add_SfrRegR(&borcon,   0x116, RegisterValue(0x80,0));
    add_SfrRegR(&fvrcon,   0x117, RegisterValue(0x00,0));
    add_SfrRegR(m_daccon0, 0x118, RegisterValue(0x00,0));
    add_SfrRegR(m_daccon1, 0x119, RegisterValue(0x00,0));
    add_SfrRegR(&apfcon2 ,  0x11c, RegisterValue(0x00,0));
    add_SfrRegR(&apfcon1 ,  0x11d, RegisterValue(0x00,0));
    add_SfrRegR(comparator.cmxcon0[2], 0x11e, RegisterValue(0x04,0));
    add_SfrRegR(comparator.cmxcon1[2], 0x11f, RegisterValue(0x00,0));
    add_SfrRegR(&ansela,   0x18c, RegisterValue(0x17,0));
    add_SfrRegR(&anselb,   0x18d, RegisterValue(0x7f,0));
    add_SfrRegR(&anselc,   0x18e, RegisterValue(0xff,0));
  get_eeprom()->get_reg_eedata()->new_name("eedatl");
  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_SfrRegR(get_eeprom()->get_reg_eeadr(),   0x191);
  add_SfrRegR(get_eeprom()->get_reg_eeadrh(),   0x192);
  add_SfrReg(get_eeprom()->get_reg_eedata(),  0x193);
  add_SfrReg(get_eeprom()->get_reg_eedatah(),  0x194);
  add_SfrRegR(get_eeprom()->get_reg_eecon1(),  0x195, RegisterValue(0,0));
  add_SfrRegR(get_eeprom()->get_reg_eecon2(),  0x196);
  add_SfrRegR(&vregcon, 0x197, RegisterValue(1,0));
  add_SfrRegR(usart.rcreg,    0x199, RegisterValue(0,0),"rcreg");
  add_SfrRegR(usart.txreg,    0x19a, RegisterValue(0,0),"txreg");
  add_SfrRegR(&usart.spbrg,   0x19b, RegisterValue(0,0),"spbrgl");
  add_SfrRegR(&usart.spbrgh,  0x19c, RegisterValue(0,0),"spbrgh");
  add_SfrRegR(&usart.rcsta,   0x19d, RegisterValue(0,0),"rcsta");
  add_SfrRegR(&usart.txsta,   0x19e, RegisterValue(2,0),"txsta");
  add_SfrRegR(&usart.baudcon, 0x19f,RegisterValue(0x40,0),"baudcon");

    add_SfrRegR(m_wpua,     0x20c, RegisterValue(0xff,0),"wpua");
    add_SfrRegR(m_wpub,     0x20d, RegisterValue(0xff,0),"wpub");
    add_SfrRegR(m_wpuc,     0x20e, RegisterValue(0xff,0),"wpuc");
    add_SfrRegR(m_wpue,     0x210, RegisterValue(0x04,0),"wpue");

  add_SfrReg(&ssp.sspbuf,  0x211, RegisterValue(0,0),"ssp1buf");
  add_SfrRegR(&ssp.sspadd,  0x212, RegisterValue(0,0),"ssp1add");
  add_SfrRegR(ssp.sspmsk, 0x213, RegisterValue(0xff,0),"ssp1msk");
  add_SfrRegR(&ssp.sspstat, 0x214, RegisterValue(0,0),"ssp1stat");
  add_SfrRegR(&ssp.sspcon,  0x215, RegisterValue(0,0),"ssp1con");
  add_SfrRegR(&ssp.sspcon2, 0x216, RegisterValue(0,0),"ssp1con2");
  add_SfrRegR(&ssp.ssp1con3, 0x217, RegisterValue(0,0),"ssp1con3");
  add_SfrReg(&ccpr1l,      0x291, RegisterValue(0,0));
  add_SfrReg(&ccpr1h,      0x292, RegisterValue(0,0));
  add_SfrRegR(&ccp1con,     0x293, RegisterValue(0,0));
  add_SfrReg(&pwm1con,     0x294, RegisterValue(0,0));
  add_SfrReg(&ccp1as,      0x295, RegisterValue(0,0));
  add_SfrReg(&pstr1con,    0x296, RegisterValue(1,0));

  add_SfrRegR(m_iocap, 0x391, RegisterValue(0,0),"iocap");
  add_SfrRegR(m_iocan, 0x392, RegisterValue(0,0),"iocan");
  add_SfrRegR(m_iocaf, 0x393, RegisterValue(0,0),"iocaf");
  m_iocaf->set_intcon(intcon);
  add_SfrRegR(m_iocbp, 0x394, RegisterValue(0,0),"iocbp");
  add_SfrRegR(m_iocbn, 0x395, RegisterValue(0,0),"iocbn");
  add_SfrRegR(m_iocbf, 0x396, RegisterValue(0,0),"iocbf");
  m_iocbf->set_intcon(intcon);
  add_SfrRegR(m_ioccp, 0x397, RegisterValue(0,0),"ioccp");
  add_SfrRegR(m_ioccn, 0x398, RegisterValue(0,0),"ioccn");
  add_SfrRegR(m_ioccf, 0x399, RegisterValue(0,0),"ioccf");
  m_ioccf->set_intcon(intcon);
  add_SfrRegR(m_iocep, 0x39d, RegisterValue(0,0),"iocep");
  add_SfrRegR(m_iocen, 0x39e, RegisterValue(0,0),"iocen");
  add_SfrRegR(m_iocef, 0x39f, RegisterValue(0,0),"iocef");
  m_iocef->set_intcon(intcon);

    add_SfrRegR(m_dac2con0, 0x591, RegisterValue(0x00,0));
    add_SfrRegR(m_dac2con1, 0x592, RegisterValue(0x00,0));
    add_SfrRegR(m_dac3con0, 0x593, RegisterValue(0x00,0));
    add_SfrRegR(m_dac3con1, 0x594, RegisterValue(0x00,0));
    add_SfrRegR(m_dac4con0, 0x595, RegisterValue(0x00,0));
    add_SfrRegR(m_dac4con1, 0x596, RegisterValue(0x00,0));

  tmr2.ssp_module[0] = &ssp;

    ssp.initialize(
        get_pir_set(),    // PIR
        &(*m_porta)[1],   // SCK
        &(*m_porta)[3],   // SS
        &(*m_porta)[0],   // SDO
        &(*m_porta)[2],   // SDI
          m_trisa,        // i2c tris port
        SSP_TYPE_MSSP1
    );
    apfcon1.set_pins(0, &ccp1con, CCPCON::CCP_PIN, &(*m_porta)[2], &(*m_porta)[5]); //CCP1/P1A
    apfcon1.set_pins(1, &ccp1con, CCPCON::PxB_PIN, &(*m_porta)[0], &(*m_porta)[4]); //P1B
    apfcon1.set_pins(2, &usart, USART_MODULE::TX_PIN, &(*m_porta)[0], &(*m_porta)[4]); //USART TX Pin
    apfcon1.set_pins(3, &t1con_g.t1gcon, 0, &(*m_porta)[4], &(*m_porta)[3]); //tmr1 gate
    apfcon1.set_pins(5, &ssp, SSP1_MODULE::SS_PIN, &(*m_porta)[3], &(*m_porta)[0]); //SSP SS
    apfcon1.set_pins(6, &ssp, SSP1_MODULE::SDO_PIN, &(*m_porta)[0], &(*m_porta)[4]); //SSP SDO
    apfcon1.set_pins(7, &usart, USART_MODULE::RX_PIN, &(*m_porta)[1], &(*m_porta)[5]); //USART RX Pin
    
    if (pir1) 
    {
        pir1->set_intcon(intcon);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    pie2.setPir(pir2);
    pie3.setPir(pir3);
    pie4.setPir(pir4);
    t2con.tmr2 = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
//  tmr2.add_ccp ( &ccp2con );
    pr2.tmr2    = &tmr2;

    ccp1as.setIOpin(0, 0, &(*m_porta)[2]);
    ccp1as.link_registers(&pwm1con, &ccp1con);

    ccp1con.setIOpin(&(*m_porta)[2], &(*m_porta)[0]);
    ccp1con.pstrcon = &pstr1con;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v1822::CCP1IF, &tmr2, &ccp1as);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;


    ansela.config(0x17, 0);
    ansela.setValidBits(0x17);
    ansela.setAdcon1(&adcon1);

    anselb.config(0x3f, 8);
    anselb.setValidBits(0x7f);
    anselb.setAdcon1(&adcon1);
    anselb.setAnsel(&ansela);
    ansela.setAnsel(&anselb);
    anselc.setValidBits(0xff);

    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setAdcon2(&adcon2);
    adcon0.setIntcon(intcon);
    adcon0.setA2DBits(12);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0x1f);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);

    adcon1.setAdcon0(&adcon0);
    adcon1.setNumberOfChannels(32); // not all channels are used
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);
    adcon1.setValidBits(0xf7);
    adcon1.setVrefHiConfiguration(0, 3);
    adcon1.setVrefLoConfiguration(0, 2);
    adcon1.set_FVR_chan(0x1f);

    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1],  &(*m_portb)[3],  &(*m_portb)[1]);
    comparator.cmxcon1[1]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1],  &(*m_portb)[3],  &(*m_portb)[1]);
    comparator.cmxcon1[2]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1],  &(*m_portb)[3],  &(*m_portb)[1]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[2], &(*m_porta)[3]);
    comparator.cmxcon1[1]->set_INpinPos(&(*m_porta)[2], &(*m_portb)[0]);
    comparator.cmxcon1[2]->set_INpinPos(&(*m_porta)[2], &(*m_portb)[4]);

    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[4]);
    comparator.cmxcon1[1]->set_OUTpin(&(*m_porta)[5]);
    comparator.cmxcon1[2]->set_OUTpin(&(*m_portb)[5]);
    comparator.cmxcon0[0]->setBitMask(0xbf);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, (1<<5)));
    comparator.cmxcon0[1]->setBitMask(0xbf);
    comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, (1<<6)));
    comparator.cmxcon0[2]->setBitMask(0xbf);
    comparator.cmxcon0[2]->setIntSrc(new InterruptSource(pir2, (1<<1)));
    comparator.cmxcon1[0]->setBitMask(0xff);
    comparator.cmxcon1[1]->setBitMask(0xff);
    comparator.cmxcon1[2]->setBitMask(0xff);

    comparator.assign_pir_set(get_pir_set());
    comparator.assign_t1gcon(&t1con_g.t1gcon);
    fvrcon.set_adcon1(&adcon1);
    fvrcon.set_daccon0(m_daccon0);
    fvrcon.set_cmModule(&comparator);
    fvrcon.set_VTemp_AD_chan(0x1d);
    fvrcon.set_FVRAD_AD_chan(0x1f);

    m_daccon0->set_adcon1(&adcon1);
    m_daccon0->set_cmModule(&comparator);
    m_daccon0->set_FVRCDA_AD_chan(0x1e);
    m_daccon0->setDACOUT(&(*m_porta)[2], &(*m_portb)[7]);

    m_dac2con0->set_adcon1(&adcon1);
    m_dac2con0->set_cmModule(&comparator);
    m_dac2con0->set_FVRCDA_AD_chan(0x1c);
    m_dac2con0->setDACOUT(&(*m_porta)[5], &(*m_portb)[7]);

    m_dac3con0->set_adcon1(&adcon1);
    m_dac3con0->set_cmModule(&comparator);
    m_dac3con0->set_FVRCDA_AD_chan(0x19);
    m_dac3con0->setDACOUT(&(*m_portb)[2], &(*m_portb)[7]);

    m_dac4con0->set_adcon1(&adcon1);
    m_dac4con0->set_cmModule(&comparator);
    m_dac4con0->set_FVRCDA_AD_chan(0x18);
    m_dac4con0->setDACOUT(&(*m_porta)[4], &(*m_portb)[7]);


    osccon->set_osctune(&osctune);
    osccon->set_oscstat(&oscstat);
    osctune.set_osccon((OSCCON *)osccon);
    osccon->write_mask = 0xfb;
}

void P16F178x::set_out_of_range_pm(uint address, uint value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
      get_eeprom()->change_rom(address - 0x2100, value);
}

void  P16F178x::create(int ram_top, int eeprom_size)
{
  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);

  osccon = new OSCCON_2(this, "osccon" );

  pic_processor::create();

  e->initialize(eeprom_size, 16, 16, 0x8000);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0xff);

  P16F178x::create_sfr_map();
  _14bit_e_processor::create_sfr_map();
}

void P16F178x::enter_sleep()
{
    tmr1l.sleep();
    osccon->sleep();
    _14bit_e_processor::enter_sleep();
}

void P16F178x::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
        tmr1l.wake();
        osccon->wake();
        _14bit_e_processor::exit_sleep();
    }
}

void P16F178x::option_new_bits_6_7(uint bits)
{
        Dprintf(("P16F178x::option_new_bits_6_7 bits=%x\n", bits));
    m_porta->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
    m_wpua->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
}

void P16F178x::oscillator_select(uint cfg_word1, bool clkout)
{
    uint mask = m_porta->getEnableMask();
    uint fosc = cfg_word1 & (FOSC0|FOSC1|FOSC2);

    osccon->set_config_irc(fosc == 4);
    osccon->set_config_xosc(fosc < 3);
    osccon->set_config_ieso(cfg_word1 & IESO);
    set_int_osc(false);
    switch(fosc)
    {
    case 0:        //LP oscillator: low power crystal
    case 1:        //XT oscillator: Crystal/resonator
    case 2:        //HS oscillator: High-speed crystal/resonator
        mask &= 0x3f;
        break;

    case 3:        //EXTRC oscillator External RC circuit connected to CLKIN pin
        mask &= 0x7f;
        if(clkout) mask &= 0xbf;
        else       mask |= 0x40;

        break;

    case 4:        //INTOSC oscillator: I/O function on CLKIN pin
        set_int_osc(true);
        
        if(clkout) mask &= 0xbf;
        else       mask |= 0x40;

        mask |= 0x80;
 
        break;

    case 5:        //ECL: External Clock, Low-Power mode (0-0.5 MHz): on CLKIN pin
        if(clkout) mask &= 0xbf;
        else       mask |= 0x40;

        mask &= 0x7f;
        break;

    case 6:        //ECM: External Clock, Medium-Power mode (0.5-4 MHz): on CLKIN pin
        if(clkout) mask &= 0xbf;
        else       mask |= 0x40;

        mask &= 0x7f;
        break;

    case 7:        //ECH: External Clock, High-Power mode (4-32 MHz): on CLKIN pin
        if(clkout) mask &= 0xbf;
        else       mask |= 0x40;

        mask &= 0x7f;
        break;
    };
    ansela.setValidBits(0x17 & mask);
    m_porta->setEnableMask(mask);
}

void P16F178x::program_memory_wp(uint mode)
{
        switch(mode)
        {
        case 3:        // no write protect
            get_eeprom()->set_prog_wp(0x0);
            break;

        case 2: // write protect 0000-01ff
            get_eeprom()->set_prog_wp(0x0200);
            break;

        case 1: // write protect 0000-03ff
            get_eeprom()->set_prog_wp(0x0400);
            break;

        case 0: // write protect 0000-07ff
            get_eeprom()->set_prog_wp(0x0800);
            break;

        default:
            printf("%s unexpected mode %u\n", __FUNCTION__, mode);
            break;
        }
}

P16F1788::P16F1788(const char *_name )
  : P16F178x(_name )
{
  comparator.cmxcon0[3] = new CMxCON0(this, "cm4con0", 3, &comparator);
  comparator.cmxcon1[3] = new CMxCON1(this, "cm4con1", 3, &comparator);
}

P16F1788::~P16F1788()
{
    remove_SfrReg(comparator.cmxcon0[3]);
    remove_SfrReg(comparator.cmxcon1[3]);
}

void P16F1788::create_iopin_map()
{
  assign_pin(1, m_porte->addPin(new IO_bi_directional_pu("porte3"),3));
  assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  assign_pin(5, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  assign_pin(6, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  assign_pin(7, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
  assign_pin(10, m_porta->addPin(new IO_bi_directional_pu("porta6"),6));
  assign_pin(9, m_porta->addPin(new IO_bi_directional_pu("porta7"),7));

  assign_pin(11, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));
  assign_pin(12, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
  assign_pin(13, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
  assign_pin(14, m_portc->addPin(new IO_bi_directional_pu("portc3"),3));
  assign_pin(15, m_portc->addPin(new IO_bi_directional_pu("portc4"),4));
  assign_pin(16, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
  assign_pin(17, m_portc->addPin(new IO_bi_directional_pu("portc6"),6));
  assign_pin(18, m_portc->addPin(new IO_bi_directional_pu("portc7"),7));

  assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  assign_pin( 20, 0);        // Vdd
  assign_pin( 19, 0);        // Vss
  assign_pin( 8, 0);        // Vss
}

Processor* P16F1788::construct(const char *name)
{
  P16F1788 *p = new P16F1788(name);

  p->create(2048, 256, 0x302b);
  p->create_invalid_registers ();

  return p;
}

void  P16F1788::create(int ram_top, int eeprom_size, int dev_id)
{
  ram_size = ram_top;
  create_iopin_map();
  P16F178x::create(ram_top, eeprom_size);
  create_sfr_map();
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(dev_id);

}

void P16F1788::create_sfr_map()
{
    add_SfrReg(comparator.cmxcon0[3], 0x11a, RegisterValue(0x04,0));
    add_SfrReg(comparator.cmxcon1[3], 0x11b, RegisterValue(0x00,0));

    adcon1.setIOPin(12, &(*m_portb)[0]);
    adcon1.setIOPin(10, &(*m_portb)[1]);
    adcon1.setIOPin(8, &(*m_portb)[2]);
    adcon1.setIOPin(9, &(*m_portb)[3]);
    adcon1.setIOPin(11, &(*m_portb)[4]);
    adcon1.setIOPin(13, &(*m_portb)[5]);

    ssp.set_sckPin(&(*m_portc)[0]);
    ssp.set_sdiPin(&(*m_portc)[1]);
    ssp.set_sdoPin(&(*m_portc)[2]);
    ssp.set_ssPin(&(*m_portc)[3]);
    ssp.set_tris(m_trisc);

    // Pin values for default APFCON
    usart.set_TXpin(&(*m_portc)[4]); // TX pin
    usart.set_RXpin(&(*m_portc)[5]);  // RX pin

    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    apfcon1.set_ValidBits(0xff);
    apfcon2.set_ValidBits(0x07);
    // pins 0,1 not used for p16f1788
    apfcon1.set_pins(2, &usart, USART_MODULE::TX_PIN, &(*m_portc)[4], &(*m_porta)[0]); //USART TX Pin
    // pin 3 defined in p12f1822
    apfcon1.set_pins(5, &ssp, SSP1_MODULE::SS_PIN, &(*m_portc)[3], &(*m_porta)[3]); //SSP SS
    apfcon1.set_pins(6, &ssp, SSP1_MODULE::SDO_PIN, &(*m_portc)[2], &(*m_porta)[4]); //SSP SDO
    apfcon1.set_pins(7, &usart, USART_MODULE::RX_PIN, &(*m_portc)[5], &(*m_porta)[1]); //USART RX Pin

    comparator.cmxcon1[3]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1],  &(*m_portb)[5],  &(*m_portb)[1]);
    comparator.cmxcon1[3]->set_INpinPos(&(*m_porta)[2], &(*m_portb)[6]);
    comparator.cmxcon1[3]->set_OUTpin(&(*m_portc)[7]);
    comparator.cmxcon0[3]->setBitMask(0xbf);
    comparator.cmxcon0[3]->setIntSrc(new InterruptSource(pir2, (1<<2)));
    comparator.cmxcon1[3]->setBitMask(0xff);

}
P16LF1788::P16LF1788(const char *_name )
  : P16F1788(_name )
{
}

P16LF1788::~P16LF1788()
{
}

Processor * P16LF1788::construct(const char *name)
{
  P16LF1788 *p = new P16LF1788(name);

  p->create(2048, 256, 0x302d);
  p->create_invalid_registers ();

  return p;
}

void  P16LF1788::create(int ram_top, int eeprom_size, int dev_id)
{
  P16F1788::create(ram_top, eeprom_size, dev_id);
}

