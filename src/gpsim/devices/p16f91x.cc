/*
   Copyright (C) 2017 Roy R. Rankin

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


//
// p16f91x
//
//  This file supports:
//    P16F917 P16F914
//    P16F916 P16F913
//

#include <stdio.h>
#include <iostream>
#include <string>

#include "p16f91x.h"
#include "pic-ioports.h"

P16F91X::P16F91X (const char *_name)
    : _14bit_processor(_name ),
      intcon_reg(this,"intcon" ),
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
      lvdcon(this, "lvdcon" ),
      ssp(this),
      adcon0(this,"adcon0" ),
      adcon1(this,"adcon1" ),
      adres(this,"adresh" ),
      adresl(this,"adresl" ),
      ansel(this, "ansel" ),
      usart(this),
      lcd_module(this, true),
      wdtcon(this, "wdtcon", 0x1f),
      osctune(this, "osctune" ),
      comparator(this)
{

    pir1 = new PIR1v4(this,"pir1", &intcon_reg, &pie1);
    pir2 = new PIR2v5(this,"pir2", &intcon_reg, &pie2);
    m_porta = new PicPortRegister(this,"porta", 8, 0xff);
    m_trisa = new PicTrisRegister(this,"trisa", m_porta, false);

    tmr0.set_cpu(this, m_porta, 4, option_reg);
    tmr0.start(0);

    m_iocb = new IOC(this, "iocb" );
    m_portb = new PicPortGRegister(this,"portb", &intcon_reg,m_iocb,8,0xff);
    m_trisb = new PicTrisRegister(this,"trisb", m_portb, false);
    m_wpub = new WPU(this, "wpub", m_portb, 0xff);

    m_portc = new PicPortRegister(this,"portc", 8, 0xff);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);

    m_porte = new PicPortRegister(this,"porte", 4, 0x0f);
    m_trise = new PicTrisRegister(this,"trise", m_porte, false, 0x7);

    osccon = new OSCCON(this, "osccon" );
    EEPROM_WIDE *e;
    e = new EEPROM_WIDE(this,pir1);
    e->initialize(256);
    e->set_intcon(&intcon_reg);
    set_eeprom_wide(e);
}

P16F91X::~P16F91X ()
{
    unassignMCLRPin();
    remove_SfrReg(&tmr0);
    remove_SfrReg(&intcon_reg);

    delete_SfrReg(m_porta);
    delete_SfrReg(m_trisa);

    delete_SfrReg(m_portb);
    delete_SfrReg(m_trisb);
    delete_SfrReg(m_wpub);
    delete_SfrReg(m_iocb);


    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);
    delete_SfrReg(m_porte);
    delete_SfrReg(m_trise);

    remove_SfrReg(&pie1);
    remove_SfrReg(&pie2);
    delete_SfrReg(pir1);
    delete_SfrReg(pir2);

    remove_SfrReg(&t1con);
    remove_SfrReg(&tmr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&pr2);

    delete_file_registers(0x20, 0x7f);
    delete_file_registers(0xa0, 0xef);
    delete_file_registers(0x120, 0x16f);

    remove_SfrReg(&adcon0);
    remove_SfrReg(&adcon1);
    remove_SfrReg(&adres);
    remove_SfrReg(&adresl);
    remove_SfrReg(&ansel);
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    remove_SfrReg(&comparator.cmcon);
    remove_SfrReg(&comparator.cmcon1);
    remove_SfrReg(&comparator.vrcon);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);
    delete_SfrReg(osccon);
    remove_SfrReg(&osctune);
    remove_SfrReg(&wdtcon);
    remove_SfrReg(get_eeprom()->get_reg_eedata());
    remove_SfrReg(get_eeprom()->get_reg_eecon1());
    remove_SfrReg(get_eeprom()->get_reg_eeadr());
    remove_SfrReg(get_eeprom()->get_reg_eecon2());
    remove_SfrReg(get_eeprom()->get_reg_eedatah());
    remove_SfrReg(get_eeprom()->get_reg_eeadrh());
    delete get_eeprom();
    remove_SfrReg(&ssp.sspbuf);
    remove_SfrReg(&ssp.sspcon);
    remove_SfrReg(&ssp.sspadd);
    remove_SfrReg(&ssp.sspstat);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&t1con);
    remove_SfrReg(&pcon);
    remove_SfrReg(&lvdcon);
    delete_SfrReg(lcd_module.lcdcon);
    delete_SfrReg(lcd_module.lcdps);
    delete_SfrReg(lcd_module.lcdSEn[0]);
    delete_SfrReg(lcd_module.lcdSEn[1]);
    delete_SfrReg(lcd_module.lcddatax[0]);
    delete_SfrReg(lcd_module.lcddatax[1]);
    delete_SfrReg(lcd_module.lcddatax[3]);
    delete_SfrReg(lcd_module.lcddatax[4]);
    delete_SfrReg(lcd_module.lcddatax[6]);
    delete_SfrReg(lcd_module.lcddatax[7]);
    delete_SfrReg(lcd_module.lcddatax[9]);
    delete_SfrReg(lcd_module.lcddatax[10]);

}
bool P16F91X::set_config_word(uint address, uint cfg_word)
{
    enum {
        CFG_FOSC0 = 1<<0,
        CFG_FOSC1 = 1<<1,
        CFG_FOSC2 = 1<<2,
        CFG_WDTE  = 1<<3,
        CFG_MCLRE = 1<<5,
        CFG_IESO  = 1<<10,
    };

    if (address == 0x2007)
    {
        uint fosc = cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2);
        uint valid_pins = m_porta->getEnableMask();
        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE)
        {
            assignMCLRPin(1);
        }
        else
        {
            unassignMCLRPin();
        }
        osccon->set_config_xosc(fosc < 3);
        osccon->set_config_irc(fosc == 4 || fosc == 5);
        osccon->set_config_ieso(cfg_word & CFG_IESO);

        switch(fosc)
        {
        case 0:  // LP oscillator: low power crystal is on RA4 and RA5
        case 1:     // XT oscillator: crystal/resonator is on RA4 and RA5
        case 2:     // HS oscillator: crystal/resonator is on RA4 and RA5
            (&(*m_porta)[6])->AnalogReq((Register *)this, true, "OSC2");
            valid_pins &= 0xcf;
            break;

        case 3:        // EC I/O on RA4 pin, CLKIN on RA5
            valid_pins &= 0xef;
            break;
            
        case 5: // INTOSC CLKOUT on RA4 pin
            (&(*m_porta)[6])->AnalogReq((Register *)this, true, "CLKOUT");
        case 4: // INTOSC
            set_int_osc(true);
            osccon->set_rc_frequency();
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
            m_trisa->setEnableMask(valid_pins & 0xf7);
        }
    }
    return true;
}
//-------------------------------------------------------------------
void P16F91X::create_sfr_map()
{

    add_SfrReg(indf,    0x00);
    alias_file_registers(0x00,0x00,0x80);
    alias_file_registers(0x00,0x00,0x100);
    alias_file_registers(0x00,0x00,0x180);


    add_SfrReg(&tmr0,   0x01);
    alias_file_registers(0x01,0x01,0x100);
    add_SfrReg(pcl,     0x02, RegisterValue(0,0));
    add_SfrReg(status,  0x03, RegisterValue(0x18,0));
    add_SfrReg(fsr,     0x04);
    alias_file_registers(0x02,0x04,0x80);
    alias_file_registers(0x02,0x04,0x100);
    alias_file_registers(0x02,0x04,0x180);

    add_SfrReg(m_porta, 0x05);
    add_SfrReg(m_portb, 0x06);
    add_SfrReg(m_wpub, 0x95, RegisterValue(0xff,0),"wpub");
    add_SfrReg(m_iocb, 0x96, RegisterValue(0xff,0),"iocb");

    alias_file_registers(0x06, 0x06, 0x100);
    add_SfrReg(m_portc, 0x07);

    add_SfrReg(pclath,  0x0a, RegisterValue(0,0));
    add_SfrReg(&intcon_reg, 0x0b, RegisterValue(0,0));
    add_SfrReg(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
    add_SfrReg(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
    add_SfrReg(&t1con, 0x10, RegisterValue(0,0));

    alias_file_registers(0x0a,0x0b,0x80);
    alias_file_registers(0x0a,0x0b,0x100);
    alias_file_registers(0x0a,0x0b,0x180);

    add_SfrReg(&tmr2,   0x11, RegisterValue(0,0));
    add_SfrReg(&t2con,  0x12, RegisterValue(0,0));
    add_SfrReg(&pr2,    0x92, RegisterValue(0xff,0));

    add_SfrReg(&adcon0, 0x1f, RegisterValue(0,0));
    add_SfrReg(&adcon1, 0x9f, RegisterValue(0,0));
    add_SfrReg(&adres,  0x1e, RegisterValue(0,0));
    add_SfrReg(&adresl, 0x9e, RegisterValue(0,0));
    add_SfrReg(&ansel, 0x91, RegisterValue(0xff,0));

    add_SfrReg(&pcon, 0x8e, RegisterValue(0x10,0));

    add_file_registers(0x20, 0x7f, 0);
    alias_file_registers(0x70,0x7f,0x80);
    alias_file_registers(0x70,0x7f,0x100);
    alias_file_registers(0x70,0x7f,0x180);

    add_file_registers(0xa0, 0xef, 0);
    add_file_registers(0x120, 0x16f, 0);

    add_SfrReg(option_reg,  0x81, RegisterValue(0xff,0));
    alias_file_registers(0x81, 0x81, 0x100);
    add_SfrReg(m_trisa, 0x85, RegisterValue(0xff,0));
    add_SfrReg(m_trisb, 0x86, RegisterValue(0xff,0));
    alias_file_registers(0x86, 0x86, 0x100);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));

    intcon = &intcon_reg;
    add_SfrReg(pir1,    0x0c, RegisterValue(0,0),"pir1");
    add_SfrReg(pir2,    0x0d, RegisterValue(0,0),"pir2");
    add_SfrReg(&pie1,   0x8c, RegisterValue(0,0), "pie1");
    add_SfrReg(&pie2,   0x8d, RegisterValue(0,0), "pie2");

    add_SfrReg(osccon, 0x8f, RegisterValue(0x60,0));
    add_SfrReg(&osctune, 0x90, RegisterValue(0,0),"osctune");
    add_SfrReg(&wdtcon, 0x105, RegisterValue(0x08,0),"wdtcon");

    add_SfrReg(lcd_module.lcdps, 0x108, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcdSEn[0], 0x11c, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcdSEn[1], 0x11d, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[0], 0x110, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[1], 0x111, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[3], 0x113, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[4], 0x114, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[6], 0x116, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[7], 0x117, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[9], 0x119, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[10], 0x11a, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcdcon, 0x107, RegisterValue(0x13,0));

    lcd_module.set_Vlcd(&(*m_portc)[0], &(*m_portc)[1], &(*m_portc)[2]);
    lcd_module.set_LCDsegn(0, &(*m_portb)[0], &(*m_portb)[1],
            &(*m_portb)[2], &(*m_portb)[3]);
    lcd_module.set_LCDsegn(4, &(*m_porta)[4], &(*m_porta)[5],
            &(*m_portc)[3], &(*m_porta)[1]);
    lcd_module.set_LCDsegn(8, &(*m_portc)[7], &(*m_portc)[6],
            &(*m_portc)[4], &(*m_portc)[5]);
    lcd_module.set_LCDsegn(12, &(*m_porta)[0], &(*m_portb)[7],
            &(*m_portb)[6], &(*m_porta)[3]);

    lcd_module.setIntSrc(new InterruptSource(pir2, (1<<4)));
    lcd_module.set_t1con(&t1con);
    osccon->set_osctune(&osctune);
    osctune.set_osccon(osccon);

    add_SfrReg(&comparator.cmcon, 0x9c, RegisterValue(0,0), "cmcon0");
    add_SfrReg(&comparator.cmcon1, 0x97, RegisterValue(0,0), "cmcon1");
    add_SfrReg(&comparator.vrcon, 0x9d, RegisterValue(0,0), "vrcon");

    // Link the comparator and voltage ref to porta
    comparator.initialize(get_pir_set(), NULL,
                          &(*m_porta)[0], &(*m_porta)[1],          // AN0 AN1
            &(*m_porta)[2], &(*m_porta)[3],          // AN2 AN3
            &(*m_porta)[4], &(*m_porta)[5]);        //OUT0 OUT1

    comparator.cmcon.set_tmrl(&tmr1l);
    comparator.cmcon1.set_tmrl(&tmr1l);

    comparator.cmcon.set_configuration(1, 0, AN0, AN3, AN0, AN3, ZERO);
    comparator.cmcon.set_configuration(2, 0, AN1, AN1, AN1, AN2, ZERO);
    comparator.cmcon.set_configuration(1, 1, AN0, AN2, AN3, AN2, NO_OUT);
    comparator.cmcon.set_configuration(2, 1, AN1, AN2, AN1, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 2, AN0, VREF, AN3, VREF, NO_OUT);
    comparator.cmcon.set_configuration(2, 2, AN1, VREF, AN2, VREF, NO_OUT);
    comparator.cmcon.set_configuration(1, 3, AN0, AN2, AN0, AN2, NO_OUT);
    comparator.cmcon.set_configuration(2, 3, AN1, AN2, AN1, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 4, AN0, AN3, AN0, AN3, NO_OUT);
    comparator.cmcon.set_configuration(2, 4, AN1, AN2, AN1, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 5, AN1, AN2, AN1, V06, NO_OUT);
    comparator.cmcon.set_configuration(1, 6, AN0, AN2, AN0, AN2, OUT0);
    comparator.cmcon.set_configuration(2, 6, AN1, AN2, AN1, AN2, OUT1);
    comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.vrcon.setValidBits(0xaf);

    pir_set_2_def.set_pir1(pir1);
    pir_set_2_def.set_pir2(pir2);

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

    ccp1con.setIOpin(&(*m_portc)[5]);
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v4::CCP1IF, &tmr2);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;


    usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
            new _TXREG(this,"txreg", &usart),
            new _RCREG(this,"rcreg", &usart));

    add_SfrReg(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
    add_SfrReg(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
    add_SfrReg(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");

    if (pir1) {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);

    if (pir2) {
        pir2->set_intcon(&intcon_reg);
        pir2->set_pie(&pie2);
    }

    //pie2.setPir(get_pir2());
    pie2.setPir(pir2);

    add_SfrReg(get_eeprom()->get_reg_eedata(),  0x10c);
    add_SfrReg(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));

    // Enable program memory reads and writes.
    get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);

    add_SfrReg(get_eeprom()->get_reg_eeadr(),   0x10d);
    add_SfrReg(get_eeprom()->get_reg_eecon2(),  0x18d);

    get_eeprom()->get_reg_eedatah()->new_name("eedath");
    add_SfrReg(get_eeprom()->get_reg_eedatah(), 0x10e);
    add_SfrReg(get_eeprom()->get_reg_eeadrh(),  0x10f);


    adcon0.setAdres(&adres);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(&intcon_reg);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(7);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);
    adcon0.Vrefhi_position = 3;
    adcon0.Vreflo_position = 2;

    adcon0.setAdresLow(&adresl);

    adcon0.setA2DBits(10);

    adcon1.setAdcon0(&adcon0);

    intcon_reg.set_pir_set(get_pir_set());

    ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
            &(*m_porta)[5],   // SS
            &(*m_portc)[5],   // SDO
            &(*m_portc)[4],   // SDI
            m_trisc,         // I2C port
            SSP_TYPE_BSSP
            );
    add_SfrReg(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
    add_SfrReg(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
    add_SfrReg(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
    add_SfrReg(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
    tmr2.ssp_module[0] = &ssp;
    add_SfrReg(&ccpr1l,  0x15, RegisterValue(0,0));
    add_SfrReg(&ccpr1h,  0x16, RegisterValue(0,0));
    add_SfrReg(&ccp1con, 0x17, RegisterValue(0,0));
    add_SfrReg(&lvdcon, 0x109, RegisterValue(4,0));
    lvdcon.setIntSrc(new InterruptSource(pir2, (1<<2)));


}
void P16F91X::option_new_bits_6_7(uint bits)
{
    m_portb->setRBPU( (bits & OPTION_REG::BIT7) == OPTION_REG::BIT7);
    m_portb->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}

void P16F91X::update_vdd()
{
    lvdcon.check_lvd();
    Processor::update_vdd();
}

void P16F91X::enter_sleep()
{
    tmr1l.sleep();
    lcd_module.sleep();
    osccon->sleep();
    _14bit_processor::enter_sleep();
}

void P16F91X::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
        tmr1l.wake();
        lcd_module.wake();
        osccon->wake();
        _14bit_processor::exit_sleep();
    }
}

//-------------------------------------------------------------------

void P16F91X_40::create_iopin_map(void)
{
    assign_pin( 1, m_porte->addPin(new IO_bi_directional("porte3"),3));
    assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
    assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
    assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
    assign_pin( 6, m_porta->addPin(new IO_bi_directional("porta4"),4));
    assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

    assign_pin( 8, m_porte->addPin(new IO_bi_directional("porte0"),0));
    assign_pin( 9, m_porte->addPin(new IO_bi_directional("porte1"),1));
    assign_pin(10, m_porte->addPin(new IO_bi_directional("porte2"),2));

    assign_pin(11, 0);
    assign_pin(12, 0);
    assign_pin(13, m_porta->addPin(new IO_bi_directional("porta7"),7));
    assign_pin(14, m_porta->addPin(new IO_bi_directional("porta6"),6));

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
}

void P16F91X_40::set_out_of_range_pm(uint address, uint value)
{

    if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
        get_eeprom()->change_rom(address - 0x2100, value);
    }
}


void P16F91X_40::create_sfr_map()
{
    P16F91X::create_sfr_map();

    add_SfrReg(m_porte, 0x09);
    add_SfrReg(m_trise, 0x89, RegisterValue(0x0f,0));
    add_SfrReg(m_portd, 0x08);
    add_SfrReg(m_trisd, 0x88, RegisterValue(0xff,0));
    adcon1.setNumberOfChannels(8);
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[3]);
    adcon1.setIOPin(4, &(*m_porta)[5]);
    adcon1.setIOPin(5, &(*m_porte)[0]);
    adcon1.setIOPin(6, &(*m_porte)[1]);
    adcon1.setIOPin(7, &(*m_porte)[2]);
    ansel.setAdcon1(&adcon1);
    ansel.setValidBits(0xff);
    ansel.config(0xff, 0);
    add_SfrReg(&ccpr2l,  0x1b, RegisterValue(0,0));
    add_SfrReg(&ccpr2h,  0x1c, RegisterValue(0,0));
    add_SfrReg(&ccp2con, 0x1d, RegisterValue(0,0));
    ccp2con.setIOpin(&(*m_portd)[2]);
    ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v5::CCP2IF, &tmr2);
    ccpr2l.ccprh  = &ccpr2h;
    ccpr2l.tmrl   = &tmr1l;
    ccpr2h.ccprl  = &ccpr2l;

    add_SfrReg(lcd_module.lcdSEn[2], 0x11e, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[2], 0x112, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[5], 0x115, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[8], 0x118, RegisterValue(0x0,0));
    add_SfrReg(lcd_module.lcddatax[11], 0x11b, RegisterValue(0x0,0));
    lcd_module.set_LCDcom(&(*m_portb)[4], &(*m_portb)[5],
            &(*m_porta)[2], &(*m_portd)[0]);
    lcd_module.set_LCDsegn(16, &(*m_portd)[3], &(*m_portd)[4],
            &(*m_portd)[5], &(*m_portd)[6]);
    lcd_module.set_LCDsegn(20, &(*m_portd)[7], &(*m_porte)[0],
            &(*m_porte)[1], &(*m_porte)[2]);
}

void P16F91X_40::create()
{
    _14bit_processor::create();

    status->rp_mask = 0x60;  // rp0 and rp1 are valid.
    indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
    indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100
}

P16F91X_40::P16F91X_40(const char *_name, const char *desc)
    : P16F91X(_name )
{
    m_portd = new PicPortRegister(this,"portd", 8, 0xff);
    m_trisd = new PicTrisRegister(this,"trisd", m_portd, false);
}

P16F91X_40::~P16F91X_40()
{
    delete_SfrReg(m_portd);
    delete_SfrReg(m_trisd);
    delete_SfrReg(lcd_module.lcddatax[2]);
    delete_SfrReg(lcd_module.lcddatax[5]);
    delete_SfrReg(lcd_module.lcddatax[8]);
    delete_SfrReg(lcd_module.lcddatax[11]);
    delete_SfrReg(lcd_module.lcdSEn[2]);
    remove_SfrReg(&ccp2con);
    remove_SfrReg(&ccpr2h);
    remove_SfrReg(&ccpr2l);
}

//========================================================================
void P16F91X_28::create_iopin_map(void)
{
    assign_pin( 1, m_porte->addPin(new IO_bi_directional("porte3"),3));
    assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
    assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
    assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
    assign_pin( 6, m_porta->addPin(new IO_bi_directional("porta4"),4));
    assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));
    assign_pin(8, 0);
    assign_pin(9, m_porta->addPin(new IO_bi_directional("porta7"),7));
    assign_pin(10, m_porta->addPin(new IO_bi_directional("porta6"),6));

    assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
    assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
    assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
    assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));

    assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
    assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
    assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
    assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));

    assign_pin(19, 0);
    assign_pin(20, 0);

    assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
    assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
    assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
    assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
    assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
    assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
    assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
    assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));
}

void P16F91X_28::set_out_of_range_pm(uint address, uint value)
{

    if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
        get_eeprom()->change_rom(address - 0x2100, value);
    }
}


void P16F91X_28::create_sfr_map()
{
    P16F91X::create_sfr_map();

    add_SfrReg(m_porte, 0x09);
    add_SfrReg(m_trise, 0x89, RegisterValue(0x04,0));
    adcon1.setNumberOfChannels(5);
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[3]);
    adcon1.setIOPin(4, &(*m_porta)[5]);
    ansel.setAdcon1(&adcon1);
    ansel.setValidBits(0x1f);
    ansel.config(0x1f, 0);
    lcd_module.set_LCDcom(&(*m_portb)[4], &(*m_portb)[5],
            &(*m_porta)[2], &(*m_porta)[3]);
}

void P16F91X_28::create()
{
    _14bit_processor::create();

    status->rp_mask = 0x60;  // rp0 and rp1 are valid.
    indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
    indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100
}

//========================================================================
P16F91X_28::P16F91X_28(const char *_name, const char *desc)
    : P16F91X(_name )
{
}

P16F91X_28::~P16F91X_28()
{
}

Processor * P16F917::construct(const char *name)
{
    P16F917 *p = new P16F917(name);

    p->create();
    p->create_invalid_registers ();
    return p;
}

void P16F917::create()
{
    P16F91X_40::create();
    P16F91X_40::create_iopin_map();
    create_sfr_map();
}

void P16F917::create_sfr_map()
{
    P16F91X_40::create_sfr_map();
    add_file_registers(0x190, 0x1ef, 0);
}

P16F917::~P16F917()
{
    delete_file_registers(0x190, 0x1ef);
}

P16F917::P16F917(const char *_name, const char *desc) : P16F91X_40(_name )
{
}

//*******************************************************
Processor * P16F916::construct(const char *name)
{
    P16F916 *p = new P16F916(name);

    p->create();
    p->create_invalid_registers ();
    return p;
}

void P16F916::create()
{
    P16F91X_28::create();
    P16F91X_28::create_iopin_map();
    create_sfr_map();
}

void P16F916::create_sfr_map()
{
    P16F91X_28::create_sfr_map();
    add_file_registers(0x190, 0x1ef, 0);
}

P16F916::~P16F916()
{
    delete_file_registers(0x190, 0x1ef);
}

P16F916::P16F916(const char *_name, const char *desc) : P16F91X_28(_name )
{
}

//*******************************************************
Processor * P16F914::construct(const char *name)
{
    P16F914 *p = new P16F914(name);

    p->create();
    p->create_invalid_registers ();
    return p;
}

void P16F914::create()
{
    P16F91X_40::create();
    P16F91X_40::create_iopin_map();
    create_sfr_map();
}

//*******************************************************
Processor * P16F913::construct(const char *name)
{
    P16F913 *p = new P16F913(name);

    p->create();
    p->create_invalid_registers ();
    return p;
}

void P16F913::create()
{
    P16F91X_28::create();
    P16F91X_28::create_iopin_map();
    create_sfr_map();

}
