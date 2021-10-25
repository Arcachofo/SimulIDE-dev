/*
 *
   Copyright (C) 2010,2015 Roy R. Rankin

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
// p16f88x
//
//  This file supports:
//    PIC16F882
//    PIC16F883
//    PIC16F884
//    PIC16F885
//    PIC16F886
//    PIC16F887
//

#include <stdio.h>
#include <iostream>
#include <string>

#include "p16f88x.h"
#include "pic-ioports.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__, __FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//========================================================================
//
// Configuration Memory for the 16F8X devices.

class Config188x : public ConfigWord
{
public:
    Config188x(P16F88x *pCpu)
        : ConfigWord("CONFIG188x", 0x3fff, pCpu, 0x2007)
    {
    }
    enum {
        FOSC0  = 1<<0,
        FOSC1  = 1<<1,
        FOSC2  = 1<<2,
        WDTEN  = 1<<3,
        PWRTEN = 1<<4,
        MCLRE  = 1<<5,

        BOREN  = 1<<8,
        BOREN1  = 1<<9,
        LVP    = 1<<12,

        CPD    = 1<<8,
        WRT0   = 1<<9,
        WRT1   = 1<<10,
        NOT_DEBUG  = 1<<11,
    };

    virtual void set(int64_t v)
    {
        Integer::set(v);
        Dprintf(("Config188x set %x\n", (int)v));
        if (m_pCpu)
        {
            m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);
        }
    }
};


//========================================================================

P16F88x::P16F88x(const char *_name)
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
      ssp(this),
      osccon(0),
      osctune(this, "osctune" ),
      wdtcon(this, "wdtcon", 1),
      usart(this),
      comparator(this),
      vrcon(this, "vrcon" ),
      srcon(this, "srcon" ),
      ansel(this,"ansel" ),
      anselh(this,"anselh" ),
      adcon0(this,"adcon0" ),
      adcon1(this,"adcon1" ),
      eccpas(this, "eccpas" ),
      pwm1con(this, "pwm1con" ),
      pstrcon(this, "pstrcon" ),
      adresh(this,"adresh" ),
      adresl(this,"adresl" )
{
    m_porta = new PicPortRegister(this,"porta", 8, 0x1f);
    m_trisa = new PicTrisRegister(this,"trisa", m_porta, false);
    m_ioc = new IOC(this, "iocb" );
    m_portb = new PicPortGRegister(this,"portb", &intcon_reg, m_ioc, 8, 0xff);
    m_trisb = new PicTrisRegister(this,"trisb", m_portb, false);
    m_portc = new PicPortRegister(this,"portc", 8, 0xff);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);
    m_porte = new PicPortRegister(this,"porte", 8, 0x0f);
    m_trise = new PicPSP_TrisRegister(this,"trise", m_porte, false);

    pir1_2_reg = new PIR1v2(this,"pir1", &intcon_reg,&pie1);
    pir2_2_reg = new PIR2v3(this,"pir2", &intcon_reg,&pie2);
    pir1 = pir1_2_reg;
    pir2 = pir2_2_reg;
    m_wpu = new WPU(this, "wpub", m_portb, 0xff);

    tmr0.set_cpu(this, m_porta, 4, option_reg);
    tmr0.start(0);
    comparator.cmxcon0[0] = new CMxCON0_V2(this, "cm1con0", 0, &comparator);
    comparator.cmxcon0[1] = new CMxCON0_V2(this, "cm2con0", 1, &comparator);
    comparator.cmxcon1[0] = new CM2CON1_V3(this, "cm2con1", 0, &comparator);
    comparator.cmxcon1[1] = comparator.cmxcon1[0];
}

P16F88x::~P16F88x()
{
    unassignMCLRPin();
    delete_file_registers(0x20, 0x7f);
    delete_file_registers(0xa0, 0xbf);

    remove_SfrReg(&tmr0);
    remove_SfrReg(&intcon_reg);
    remove_SfrReg(&pie2);
    remove_SfrReg(&pie1);
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&pcon);
    remove_SfrReg(&t1con);
    remove_SfrReg(&tmr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&pr2);
    remove_SfrReg(get_eeprom()->get_reg_eedata());
    remove_SfrReg(get_eeprom()->get_reg_eeadr());
    remove_SfrReg(get_eeprom()->get_reg_eedatah());
    remove_SfrReg(get_eeprom()->get_reg_eeadrh());
    remove_SfrReg(get_eeprom()->get_reg_eecon1());
    remove_SfrReg(get_eeprom()->get_reg_eecon2());
    delete get_eeprom();

    remove_SfrReg(&intcon_reg);
    remove_SfrReg(osccon);
    remove_SfrReg(&osctune);
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    remove_SfrReg(&usart.spbrgh);
    remove_SfrReg(&usart.baudcon);
    remove_SfrReg(&vrcon);
    remove_SfrReg(&srcon);
    remove_SfrReg(&wdtcon);
    remove_SfrReg(&ccpr2l);
    remove_SfrReg(&ccpr2h);
    remove_SfrReg(&ccp2con);
    remove_SfrReg(&adresl);
    remove_SfrReg(&adresh);
    remove_SfrReg(&ansel);
    remove_SfrReg(&anselh);
    remove_SfrReg(&adcon0);
    remove_SfrReg(&adcon1);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&ccpr2l);
    remove_SfrReg(&ccpr2h);
    remove_SfrReg(&ccp2con);
    remove_SfrReg(&pwm1con);
    remove_SfrReg(&pstrcon);
    remove_SfrReg(&eccpas);
    remove_SfrReg(&ssp.sspcon2);
    remove_SfrReg(&ssp.sspbuf);
    remove_SfrReg(&ssp.sspcon);
    remove_SfrReg(&ssp.sspadd);
    remove_SfrReg(&ssp.sspstat);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);
    remove_SfrReg(comparator.cmxcon0[0]);
    remove_SfrReg(comparator.cmxcon0[1]);
    remove_SfrReg(comparator.cmxcon1[1]);

    delete_SfrReg(m_porta);
    delete_SfrReg(m_trisa);
    delete_SfrReg(m_portb);
    delete_SfrReg(m_trisb);
    delete_SfrReg(m_porte);
    delete_SfrReg(m_trise);
    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);

    delete_SfrReg(pir1);
    delete_SfrReg(pir2);
    delete_SfrReg(m_wpu);
    delete_SfrReg(m_ioc);
}

void P16F88x::create_iopin_map()
{
    fprintf(stderr, "%s should be defined at a higer level\n", __FUNCTION__);
}

void P16F88x::create_sfr_map()
{
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

    add_SfrReg(m_portb, 0x06);
    add_SfrReg(m_trisb, 0x86, RegisterValue(0xff,0));

    add_SfrReg(pclath,  0x0a, RegisterValue(0,0));

    add_SfrReg(&intcon_reg, 0x0b, RegisterValue(0,0));
    //alias_file_registers(0x0a,0x0b,0x80); //Already donw

    intcon = &intcon_reg;

    pir_set_2_def.set_pir1(pir1);
    pir_set_2_def.set_pir2(pir2);

    add_SfrReg(m_porte, 0x09);
    add_SfrReg(m_trise, 0x89, RegisterValue(0xff,0));
    add_SfrReg(m_portc, 0x07);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));

    add_file_registers(0x20, 0x7f, 0);
    add_file_registers(0xa0, 0xbf, 0);

    alias_file_registers(0x70,0x7f,0x80);
    alias_file_registers(0x70,0x7f,0x100);
    alias_file_registers(0x70,0x7f,0x180);

    add_SfrReg(get_pir2(),   0x0d, RegisterValue(0,0),"pir2");
    add_SfrReg(&pie2,   0x8d, RegisterValue(0,0));

    pir_set_2_def.set_pir2(pir2);

    pie2.setPir(get_pir2());
    alias_file_registers(0x00,0x04,0x100);
    alias_file_registers(0x80,0x84,0x100);
    alias_file_registers(0x06,0x06,0x100);
    alias_file_registers(0x86,0x86,0x100);

    add_SfrReg(pir1,   0x0c, RegisterValue(0,0),"pir1");
    add_SfrReg(&pie1,   0x8c, RegisterValue(0,0));

    add_SfrReg(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
    add_SfrReg(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

    add_SfrReg(&pcon,   0x8e, RegisterValue(0,0),"pcon");

    add_SfrReg(&t1con,  0x10, RegisterValue(0,0));
    add_SfrReg(&tmr2,   0x11, RegisterValue(0,0));
    add_SfrReg(&t2con,  0x12, RegisterValue(0,0));
    add_SfrReg(&pr2,    0x92, RegisterValue(0xff,0));

    get_eeprom()->get_reg_eedata()->new_name("eedat");
    get_eeprom()->get_reg_eedatah()->new_name("eedath");
    add_SfrReg(get_eeprom()->get_reg_eedata(),  0x10c);
    add_SfrReg(get_eeprom()->get_reg_eeadr(),   0x10d);
    add_SfrReg(get_eeprom()->get_reg_eedatah(),  0x10e);
    add_SfrReg(get_eeprom()->get_reg_eeadrh(),   0x10f);
    add_SfrReg(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
    get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);
    add_SfrReg(get_eeprom()->get_reg_eecon2(),  0x18d);

    alias_file_registers(0x0a,0x0b,0x080);
    alias_file_registers(0x0a,0x0b,0x100);
    alias_file_registers(0x0a,0x0b,0x180);

    intcon_reg.set_pir_set(get_pir_set());

    add_SfrReg(osccon, 0x8f, RegisterValue(0x60,0),"osccon");
    add_SfrReg(&osctune, 0x90, RegisterValue(0,0),"osctune");

    osccon->set_osctune(&osctune);
    osctune.set_osccon(osccon);

    usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
            new _TXREG(this,"txreg", &usart),
            new _RCREG(this,"rcreg", &usart));

    add_SfrReg(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
    add_SfrReg(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg(&usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
    add_SfrReg(&usart.baudcon,  0x187,RegisterValue(0x40,0),"baudctl");
    add_SfrReg(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
    add_SfrReg(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
    usart.set_eusart(true);
    comparator.assign_tmr1l(&tmr1l);
    comparator.cmxcon1[1]->set_vrcon(&vrcon);

    add_SfrReg(comparator.cmxcon0[0], 0x107, RegisterValue(0,0), "cm1con0");
    add_SfrReg(comparator.cmxcon0[1], 0x108, RegisterValue(0,0), "cm2con0");
    add_SfrReg(comparator.cmxcon1[1], 0x109, RegisterValue(2,0), "cm2con1");
    add_SfrReg(&vrcon, 0x97, RegisterValue(0,0),"vrcon");
    add_SfrReg(&srcon, 0x185, RegisterValue(0,0),"srcon");
    add_SfrReg(&wdtcon, 0x105, RegisterValue(0x08,0),"wdtcon");
    add_SfrReg(&adresl,  0x9e, RegisterValue(0,0));
    add_SfrReg(&adresh,  0x1e, RegisterValue(0,0));
    add_SfrReg(&ansel, 0x188, RegisterValue(0xff,0));
    add_SfrReg(&anselh, 0x189, RegisterValue(0xff,0));
    add_SfrReg(&adcon0, 0x1f, RegisterValue(0,0));
    add_SfrReg(&adcon1, 0x9f, RegisterValue(0,0));
    add_SfrReg(m_wpu, 0x95, RegisterValue(0xff,0));
    add_SfrReg(m_ioc, 0x96, RegisterValue(0,0));

    ansel.setAdcon1(&adcon1);
    ansel.setAnselh(&anselh);
    anselh.setAdcon1(&adcon1);
    anselh.setAnsel(&ansel);
    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(&intcon_reg);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0xf);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);

    adcon1.setValidBits(0xb0);
    adcon1.setNumberOfChannels(14);
    adcon1.setValidCfgBits(ADCON1::VCFG0 | ADCON1::VCFG1 , 4);
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[3]);
    adcon1.setIOPin(4, &(*m_porta)[4]);
    adcon1.setIOPin(8, &(*m_portb)[2]);
    adcon1.setIOPin(9, &(*m_portb)[3]);
    adcon1.setIOPin(10, &(*m_portb)[1]);
    adcon1.setIOPin(11, &(*m_portb)[4]);
    adcon1.setIOPin(12, &(*m_portb)[0]);
    adcon1.setIOPin(13, &(*m_portb)[5]);

    // set a2d modes where an3 is Vref+
    adcon1.setVrefHiConfiguration(1, 3);
    adcon1.setVrefHiConfiguration(3, 3);

    // set a2d modes where an2 is Vref-
    adcon1.setVrefLoConfiguration(2, 2);
    adcon1.setVrefLoConfiguration(3, 2);

    vrcon.setValidBits(0xff); // All bits settable

    add_SfrReg(&ccpr1l, 0x15, RegisterValue(0,0));
    add_SfrReg(&ccpr1h, 0x16, RegisterValue(0,0));
    add_SfrReg(&ccp1con, 0x17, RegisterValue(0,0));
    add_SfrReg(&ccpr2l, 0x1b, RegisterValue(0,0));
    add_SfrReg(&ccpr2h, 0x1c, RegisterValue(0,0));
    add_SfrReg(&ccp2con, 0x1d, RegisterValue(0,0));
    add_SfrReg(&pwm1con, 0x9b, RegisterValue(0,0));
    add_SfrReg(&pstrcon, 0x9d, RegisterValue(1,0));
    add_SfrReg(&eccpas, 0x9c, RegisterValue(0,0));
    eccpas.setIOpin(0, 0, &(*m_portb)[0]);
    eccpas.link_registers(&pwm1con, &ccp1con);
    ssp.sspmsk = new _SSPMSK(this, "ssp1msk");
    add_SfrReg(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
    add_SfrReg(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
    add_SfrReg(&ssp.sspcon2,  0x91, RegisterValue(0,0),"sspcon2");
    add_SfrReg(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
    add_SfrReg(ssp.sspmsk,  0x93, RegisterValue(0xff,0), "sspmsk", false);
    add_SfrReg(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
    tmr2.ssp_module[0] = &ssp;

    ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
            &(*m_porta)[5],   // SS
            &(*m_portc)[5],   // SDO
            &(*m_portc)[4],    // SDI
            m_trisc,          // i2c tris port
            SSP_TYPE_SSP
            );
    tmr1l.tmrh = &tmr1h;
    tmr1l.t1con = &t1con;
    tmr1h.tmrl  = &tmr1l;

    t1con.tmrl  = &tmr1l;

    t2con.tmr2  = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
    tmr2.add_ccp ( &ccp2con );
    pr2.tmr2    = &tmr2;

    tmr1l.setIOpin(&(*m_portc)[0]);
    ccp1con.setBitMask(0xff);
    ccp1con.pstrcon = &pstrcon;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;

    ccp2con.setIOpin(&(*m_portc)[1]);
    ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v3::CCP2IF, &tmr2);
    ccpr2l.ccprh  = &ccpr2h;
    ccpr2l.tmrl   = &tmr1l;
    ccpr2h.ccprl  = &ccpr2l;

    if (pir1) {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v3::TMR1IF));

    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[4], &(*m_porta)[5]);
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1],
            &(*m_portb)[3],&(*m_portb)[1]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[3], &(*m_porta)[2]);
    comparator.cmxcon1[0]->setBitMask(0x33);
    comparator.cmxcon0[0]->setBitMask(0xb7);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, PIR2v2::C1IF));
    comparator.cmxcon0[1]->setBitMask(0xb7);
    comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, PIR2v2::C2IF));
}

void P16F88x::option_new_bits_6_7(uint bits)
{
    Dprintf(("P18F88x::option_new_bits_6_7 bits=%x\n", bits));
    m_portb->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
    m_wpu->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
}

void P16F88x::set_out_of_range_pm(uint address, uint value)
{
    if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
        get_eeprom()->change_rom(address - 0x2100, value);
    }
}

bool P16F88x::set_config_word(uint address, uint cfg_word)
{
    enum {
        CFG_FOSC0 = 1<<0,
        CFG_FOSC1 = 1<<1,
        CFG_FOSC2 = 1<<4,
        CFG_MCLRE = 1<<5,
        CFG_CCPMX = 1<<12
    };
    // Let the base class do most of the work:
    if (address == 0x2007)
    {
        pic_processor::set_config_word(address, cfg_word);

        uint valid_pins = m_porta->getEnableMask();

        set_int_osc(false);
        // Careful these bits not adjacent
        switch(cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2)) {

        case 0:  // LP oscillator: low power crystal is on RA6 and RA7
        case 1:     // XT oscillator: crystal/resonator is on RA6 and RA7
        case 2:     // HS oscillator: crystal/resonator is on RA6 and RA7
            break;

        case 0x13:  // ER oscillator: RA6 is CLKOUT, resistor (?) on RA7
            break;

        case 3:     // EC:  RA6 is an I/O, RA7 is a CLKIN
        case 0x12:  // ER oscillator: RA6 is an I/O, RA7 is a CLKIN
            valid_pins =  (valid_pins & 0x7f)|0x40;
            break;

        case 0x10:  // INTRC: Internal Oscillator, RA6 and RA7 are I/O's
            set_int_osc(true);
            valid_pins |= 0xc0;
            break;

        case 0x11:  // INTRC: Internal Oscillator, RA7 is an I/O, RA6 is CLKOUT
            set_int_osc(true);
            valid_pins = (valid_pins & 0xbf)|0x80;
            break;

        }
        // If the /MCLRE bit is set then RE3 is the MCLR pin, otherwise it's
        // a general purpose I/O pin.

        if ((cfg_word & CFG_MCLRE))
        {
            assignMCLRPin(1);
        }
        else
        {
            unassignMCLRPin();
        }

        if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
        {
            m_porta->setEnableMask(valid_pins);
            m_porta->setTris(m_trisa);
        }
        return true;
    }
    else if (address == 0x2008 )
    {
        //cout << "p16f88x 0x" << hex << address << " config word2 0x" << cfg_word << '\n';
    }
    return false;
}

void P16F88x::create_config_memory()
{
    m_configMemory = new ConfigMemory(this,2);
    m_configMemory->addConfigWord(0,new Config188x(this));
    m_configMemory->addConfigWord(1,new ConfigWord("CONFIG2", 0, this,0x2008));
    wdt.initialize(true); // default WDT enabled
    wdt.set_timeout(0.000035);
    set_config_word(0x2007, 0x3fff);

}

void  P16F88x::create(int eesize)
{
    create_iopin_map();

    _14bit_processor::create();
    osccon = new OSCCON(this, "osccon" );

    EEPROM_WIDE *e;
    e = new EEPROM_WIDE(this,pir2);
    e->initialize(eesize);
    e->set_intcon(&intcon_reg);
    set_eeprom_wide(e);

    status->rp_mask = 0x60;  // rp0 and rp1 are valid.
    indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
    indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

    P16F88x::create_sfr_map();
}

//========================================================================
//
Processor * P16F882::construct(const char *name)
{
    P16F882 *p = new P16F882(name);

    p->P16F88x::create(128);
    p->P16F882::create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F882::P16F882(const char *_name )
    : P16F88x(_name )
{
    m_porta->setEnableMask(0xff);
}

void P16F882::create_iopin_map(void)
{
    assign_pin(1, m_porte->addPin(new IO_bi_directional("porte3"),3));
    assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
    assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
    assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
    assign_pin( 6, m_porta->addPin( new IOPIN("porta4", OPEN_COLLECTOR),4) );
    assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));
    assign_pin(8, 0);
    assign_pin( 9, m_porta->addPin(new IO_bi_directional("porta7"),7));
    assign_pin( 10, m_porta->addPin(new IO_bi_directional("porta6"),6));
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

void P16F882::create_sfr_map()
{
    ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F883 
//
Processor * P16F883::construct(const char *name)
{
    P16F883 *p = new P16F883(name);

    p->P16F88x::create(256);
    p->P16F883::create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F883::P16F883(const char *_name )
    : P16F882(_name )
{
    m_porta->setEnableMask(0xff);
}

P16F883::~P16F883()
{
    delete_file_registers(0xc0,0xef);
    delete_file_registers(0x120,0x16f);
}

void P16F883::create_sfr_map()
{
    add_file_registers(0xc0,0xef,0);
    add_file_registers(0x120,0x16f,0);
    ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F886 
//

Processor * P16F886::construct(const char *name)
{
    P16F886 *p = new P16F886(name);

    p->P16F88x::create(256);
    p->P16F886::create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F886::P16F886(const char *_name )
    : P16F882(_name )
{
    m_porta->setEnableMask(0xff);
}

P16F886::~P16F886()
{
    delete_file_registers(0xc0,0xef);
    delete_file_registers(0x120,0x16f);
    delete_file_registers(0x190,0x1ef);
}

void P16F886::create_sfr_map()
{
    add_file_registers(0xc0,0xef,0);
    add_file_registers(0x120,0x16f,0);
    add_file_registers(0x190,0x1ef,0);
    ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F887 
//

Processor * P16F887::construct(const char *name)
{
    P16F887 *p = new P16F887(name);

    p->P16F88x::create(256);
    p->P16F887::create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F887::P16F887(const char *_name )
    : P16F884(_name )
{
}

P16F887::~P16F887()
{
    delete_file_registers(0x110,0x11f);
    delete_file_registers(0x190,0x1ef);
}

void P16F887::create_sfr_map()
{
    add_file_registers(0xc0,0xef,0);
    add_file_registers(0x110,0x16f,0);
    //add_file_registers(0x110,0x11f,0);
    add_file_registers(0x190,0x1ef,0);

    add_SfrReg(m_portd, 0x08);
    add_SfrReg(m_trisd, 0x88, RegisterValue(0xff,0));

    ccp1con.setIOpin(&(*m_portc)[2], &(*m_portd)[5], &(*m_portd)[6], &(*m_portd)[7]);
    adcon1.setIOPin(5, &(*m_porte)[0]);
    adcon1.setIOPin(6, &(*m_porte)[1]);
    adcon1.setIOPin(7, &(*m_porte)[2]);
}

//========================================================================
//
Processor * P16F884::construct(const char *name)
{
    P16F884 *p = new P16F884(name);

    p->P16F88x::create(256);
    p->P16F884::create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F884::P16F884(const char *_name )
    : P16F88x(_name )
{
    m_porta->setEnableMask(0xff);
    m_trisa->setEnableMask(0xff);

    m_portd = new PicPSP_PortRegister(this,"portd", 8,0xff);
    m_trisd = new PicTrisRegister(this,"trisd", (PicPortRegister *)m_portd, false);
}

P16F884::~P16F884()
{
    delete_file_registers(0xc0,0xef);
    delete_file_registers(0x120,0x16f);

    delete_SfrReg(m_portd);
    delete_SfrReg(m_trisd);
}

//------------------------------------------------------------------------
//
void P16F884::create_iopin_map(void)
{
    assign_pin(1, m_porte->addPin(new IO_bi_directional("porte3"),3));
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
    assign_pin( 13, m_porta->addPin(new IO_bi_directional("porta7"),7));
    assign_pin( 14, m_porta->addPin(new IO_bi_directional("porta6"),6));
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

void P16F884::create_sfr_map()
{
    add_file_registers(0xc0,0xef,0);
    add_file_registers(0x120,0x16f,0);

    add_SfrReg(m_portd, 0x08);
    add_SfrReg(m_trisd, 0x88, RegisterValue(0xff,0));

    ccp1con.setIOpin(&(*m_portc)[2], &(*m_portd)[5], &(*m_portd)[6], &(*m_portd)[7]);
    adcon1.setIOPin(5, &(*m_porte)[0]);
    adcon1.setIOPin(6, &(*m_porte)[1]);
    adcon1.setIOPin(7, &(*m_porte)[2]);
}
//------------------------------------------------------------------------
//
//

class ConfigF631 : public ConfigWord
{
public:
    ConfigF631(P16F631 *pCpu)
        : ConfigWord("CONFIG", 0x3fff, pCpu, 0x2007)
    {
        Dprintf(("ConfigF631::ConfigF631 %p\n", m_pCpu));
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

        const char *OSCdesc[8] = {
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
P16F631::P16F631(const char *_name )
    : _14bit_processor(_name ),
      t1con(this, "t1con" ),
      pie1(this,"pie1" ),
      pie2(this,"pie2" ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      osctune(this, "osctune" ),
      pcon(this, "pcon" ),
      wdtcon(this, "wdtcon", 0x1f),
      osccon(0),
      vrcon(this, "vrcon" ),
      srcon(this, "srcon" ),
      ansel(this,"ansel" ),
      comparator(this),
      adcon0(this,"adcon0" ),
      adcon1(this,"adcon1" ),

      intcon_reg(this,"intcon" )
{
    pir1_2_reg = new PIR1v2(this,"pir1", &intcon_reg,&pie1);
    pir1 = pir1_2_reg;
    pir2_3_reg = new PIR2v3(this,"pir2", &intcon_reg,&pie2);
    pir2 = pir2_3_reg;

    m_ioca = new IOC(this, "ioca" );
    m_iocb = new IOC(this, "iocb" );

    m_porta = new PicPortGRegister(this,"porta", &intcon_reg, m_ioca, 8,0x3f);
    m_trisa = new PicTrisRegister(this,"trisa", m_porta, false, 0x37);

    m_portb = new PicPortGRegister(this,"portb", &intcon_reg, m_iocb, 8,0xf0);
    m_trisb = new PicTrisRegister(this,"trisb", m_portb, false);

    m_wpua = new WPU(this, "wpua", m_porta, 0x37);
    m_wpub = new WPU(this, "wpub", m_portb, 0xf0);
    tmr0.set_cpu(this, m_porta, 4, option_reg);
    tmr0.start(0);

    m_portc = new PicPortRegister(this,"portc", 8,0xff);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);

    comparator.cmxcon0[0] = new CMxCON0_V2(this, "cm1con0", 0, &comparator);
    comparator.cmxcon0[1] = new CMxCON0_V2(this, "cm2con0", 1, &comparator);
    comparator.cmxcon1[0] = new CM2CON1_V4(this, "cm2con1", 0, &comparator);
    comparator.cmxcon1[1] = comparator.cmxcon1[0];
}

P16F631::~P16F631()
{
    unassignMCLRPin();
    delete_file_registers(0x40, 0x7f);
    remove_SfrReg(comparator.cmxcon0[0]);
    remove_SfrReg(comparator.cmxcon0[1]);
    remove_SfrReg(comparator.cmxcon1[1]);

    remove_SfrReg(get_eeprom()->get_reg_eedata());
    remove_SfrReg(get_eeprom()->get_reg_eeadr());
    remove_SfrReg(get_eeprom()->get_reg_eecon1());
    remove_SfrReg(get_eeprom()->get_reg_eecon2());
    remove_SfrReg(&tmr0);
    remove_SfrReg(&vrcon);
    remove_SfrReg(&ansel);
    remove_SfrReg(&srcon);
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&t1con);
    remove_SfrReg(&pcon);
    remove_SfrReg(&wdtcon);
    remove_SfrReg(osccon);
    remove_SfrReg(&pie1);
    remove_SfrReg(&pie2);
    remove_SfrReg(&intcon_reg);
    remove_SfrReg(&osctune);
    delete_SfrReg(pir2);
    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);

    delete_SfrReg(m_portb);
    delete_SfrReg(m_trisb);
    delete_SfrReg(m_porta);
    delete_SfrReg(m_trisa);
    delete_SfrReg(m_ioca);
    delete_SfrReg(m_iocb);
    delete_SfrReg(m_wpua);
    delete_SfrReg(m_wpub);
    delete_SfrReg(pir1_2_reg);
    delete e;
}
void P16F631::create_iopin_map(void)
{
    assign_pin(1, 0);        // Vdd
    assign_pin( 2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
    assign_pin( 3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
    assign_pin( 4, m_porta->addPin(new IOPIN("porta3"),3));
    assign_pin( 5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
    assign_pin( 6, m_portc->addPin(new IO_bi_directional("portc4"),4));
    assign_pin( 7, m_portc->addPin(new IO_bi_directional("portc3"),3));
    assign_pin( 8, m_portc->addPin(new IO_bi_directional("portc6"),6));
    assign_pin( 9, m_portc->addPin(new IO_bi_directional("portc7"),7));
    assign_pin(10, m_portb->addPin(new IO_bi_directional("portb7"),7));
    assign_pin(11, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
    assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
    assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
    assign_pin(14, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
    assign_pin(15, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
    assign_pin(16, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));
    assign_pin(17, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
    assign_pin(18, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
    assign_pin(19, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));

    assign_pin(20, 0); //VSS

    tmr1l.setIOpin(&(*m_portc)[0]);
}
Processor * P16F631::construct(const char *name)
{
    P16F631 *p = new P16F631(name);

    p->create(128);
    p->create_invalid_registers ();

    return p;
}

void P16F631::create(int eesize)
{
    create_iopin_map();

    _14bit_processor::create();
    osccon = new OSCCON(this, "osccon" );

    e = new EEPROM_WIDE(this,pir2);
    e->initialize(eesize);
    e->set_intcon(&intcon_reg);
    set_eeprom_wide(e);

    status->rp_mask = 0x60;  // rp0 and rp1 are valid.
    indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
    indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

    P16F631::create_sfr_map();
}

//-------------------------------------------------------------------
void P16F631::create_sfr_map()
{
    pir_set_2_def.set_pir1(pir1);
    pir_set_2_def.set_pir2(pir2);

    add_file_registers(0x40, 0x7f, 0);
    alias_file_registers(0x70, 0x7f, 0x80);
    alias_file_registers(0x70, 0x7f, 0x100);
    alias_file_registers(0x70, 0x7f, 0x180);

    add_SfrReg(indf,    0x00);
    alias_file_registers(0x00,0x00,0x80);
    alias_file_registers(0x00,0x00,0x100);
    alias_file_registers(0x00,0x00,0x180);

    add_SfrReg(&tmr0,   0x01);
    alias_file_registers(0x01,0x01,0x100);
    add_SfrReg(option_reg,  0x81, RegisterValue(0xff,0));
    alias_file_registers(0x81,0x81,0x100);

    add_SfrReg(pcl,     0x02, RegisterValue(0,0));
    add_SfrReg(status,  0x03, RegisterValue(0x18,0));
    add_SfrReg(fsr,     0x04);
    alias_file_registers(0x02,0x04,0x80);
    alias_file_registers(0x02,0x04,0x100);
    alias_file_registers(0x02,0x04,0x180);

    add_SfrReg(m_porta, 0x05);
    add_SfrReg(m_trisa, 0x85, RegisterValue(0x3f,0));

    add_SfrReg(m_portb, 0x06);
    add_SfrReg(m_trisb, 0x86, RegisterValue(0xf0,0));

    add_SfrReg(m_portc, 0x07);
    add_SfrReg(m_trisc, 0x87, RegisterValue(0xff,0));
    alias_file_registers(0x05,0x07,0x100);
    alias_file_registers(0x85,0x87,0x100);

    add_SfrReg(pclath,  0x0a, RegisterValue(0,0));
    add_SfrReg(&intcon_reg, 0x00b, RegisterValue(0,0));

    alias_file_registers(0x0a,0x0b,0x80);
    alias_file_registers(0x0a,0x0b,0x100);
    alias_file_registers(0x0a,0x0b,0x180);
    add_SfrReg(pir1, 0x0c, RegisterValue(0,0));
    add_SfrReg(pir2, 0x0d, RegisterValue(0,0));
    add_SfrReg(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
    add_SfrReg(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
    add_SfrReg(&t1con, 0x10, RegisterValue(0,0));
    add_SfrReg(&pcon, 0x8e, RegisterValue(0,0));
    add_SfrReg(&wdtcon, 0x97, RegisterValue(0x08,0));
    add_SfrReg(osccon, 0x8f, RegisterValue(0x60,0));

    add_SfrReg(&vrcon, 0x118, RegisterValue(0,0),"vrcon");
    add_SfrReg(comparator.cmxcon0[0], 0x119, RegisterValue(0,0), "cm1con0");
    add_SfrReg(comparator.cmxcon0[1], 0x11a, RegisterValue(0,0), "cm2con0");
    add_SfrReg(comparator.cmxcon1[1], 0x11b, RegisterValue(2,0), "cm2con1");
    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2], &(*m_portc)[4]);
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1],
            &(*m_portc)[2], &(*m_portc)[3]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[0], &(*m_portc)[0]);
    comparator.cmxcon1[0]->setBitMask(0x03);
    comparator.cmxcon0[0]->setBitMask(0xb7);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, PIR2v2::C1IF));
    comparator.cmxcon0[1]->setBitMask(0xb7);
    comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, PIR2v2::C2IF));
    comparator.cmxcon1[0]->set_vrcon(&vrcon);
    comparator.cmxcon1[1] = comparator.cmxcon1[0];
    comparator.assign_tmr1l(&tmr1l);

    add_SfrReg(&ansel, 0x11e, RegisterValue(0xff,0));
    add_SfrReg(&srcon, 0x19e, RegisterValue(0,0),"srcon");

    ansel.setAdcon1(&adcon1);
    ansel.setValidBits(0xff);

    adcon1.setNumberOfChannels(12);
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(4, &(*m_portc)[0]);

    adcon1.setIOPin(5, &(*m_portc)[1]);
    adcon1.setIOPin(6, &(*m_portc)[2]);
    adcon1.setIOPin(7, &(*m_portc)[3]);
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
    add_SfrReg(&pie2,   0x8d, RegisterValue(0,0));
    if (pir1) {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    pie2.setPir(pir2);

    get_eeprom()->get_reg_eedata()->new_name("eedat");
    add_SfrReg(get_eeprom()->get_reg_eedata(),  0x10c);
    add_SfrReg(get_eeprom()->get_reg_eeadr(),   0x10d);
    add_SfrReg(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
    add_SfrReg(get_eeprom()->get_reg_eecon2(),  0x18d);
    add_SfrReg(m_wpua, 0x95, RegisterValue(0x37,0),"wpua");
    add_SfrReg(m_wpub, 0x115, RegisterValue(0xf0,0),"wpub");
    add_SfrReg(m_ioca, 0x96, RegisterValue(0,0),"ioca");
    add_SfrReg(m_iocb, 0x116, RegisterValue(0,0),"iocb");
    add_SfrReg(&osctune, 0x90, RegisterValue(0,0),"osctune");

    osccon->set_osctune(&osctune);
    osctune.set_osccon(osccon);
}
//-------------------------------------------------------------------
void P16F631::option_new_bits_6_7(uint bits)
{
    m_wpua->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
    m_wpub->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
    m_porta->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}
//-------------------------------------------------------------------
void P16F631::create_config_memory()
{
    m_configMemory = new ConfigMemory(this,1);
    m_configMemory->addConfigWord(0,new ConfigF631(this));
    wdt.initialize(true); // default WDT enabled
    wdt.set_timeout(0.000035);
    set_config_word(0x2007, 0x3fff);

};

//-------------------------------------------------------------------
bool P16F631::set_config_word(uint address, uint cfg_word)
{
    enum {
        CFG_FOSC0 = 1<<0,
        CFG_FOSC1 = 1<<1,
        CFG_FOSC2 = 1<<2,
        CFG_WDTE  = 1<<3,
        CFG_MCLRE = 1<<5,
        CFG_IESO  = 1<<10,
    };

    if(address == config_word_address())
    {
        uint valid_pins = m_porta->getEnableMask();

        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE)
        {
            assignMCLRPin(4);
        }
        else
        {
            unassignMCLRPin();
        }

        wdt.initialize((cfg_word & CFG_WDTE) == CFG_WDTE);

        set_int_osc(false);

        // AnalogReq is used so ADC does not change clock names
        // set_config_word is first called with default and then
        // often called a second time. the following call is to
        // reset porta so next call to AnalogReq sill set the pin name
        //
        (&(*m_porta)[4])->AnalogReq((Register *)this, false, "porta4");
        valid_pins |= 0x20;

        uint fosc = cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2);
        if (osccon)
        {
            osccon->set_config_xosc(fosc < 3);
            osccon->set_config_irc(fosc == 4 || fosc == 5);
            osccon->set_config_ieso(cfg_word & CFG_IESO);
        }

        switch(fosc)
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
        return(true);
    }
    return false;
}

//========================================================================
//
// Pic 16F684
//
P16F684::P16F684(const char *_name )
    : _14bit_processor(_name ),
      comparator(this),
      t1con(this, "t1con" ),
      t2con(this, "t2con" ),
      pie1(this,"pie1" ),
      pr2(this, "pr2" ),
      tmr2(this, "tmr2" ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      osctune(this, "osctune" ),
      pcon(this, "pcon" ),
      wdtcon(this, "wdtcon", 0x1f),
      osccon(0),
      ansel(this, "ansel" ),
      adcon0(this, "adcon0" ),
      adcon1(this, "adcon1" ),
      adresh(this, "adresh" ),
      adresl(this, "adresl" ),
      ccp1con(this, "ccp1con" ),
      ccpr1l(this, "ccpr1l" ),
      ccpr1h(this, "ccpr1h" ),
      eccpas(this, "eccpas" ),
      pwm1con(this, "pwm1con" ),
      pstrcon(this, "pstrcon" ),
      intcon_reg(this,"intcon" )
{
    pir1_3_reg = new PIR1v3(this,"pir1", &intcon_reg,&pie1);
    pir1 = pir1_3_reg;
    pir1->valid_bits = pir1->writable_bits = 0xff;

    m_ioca = new IOC(this, "ioca" );

    m_porta = new PicPortGRegister(this,"porta", &intcon_reg, m_ioca, 8,0x3f);
    m_trisa = new PicTrisRegister(this,"trisa", m_porta, false);

    m_wpua = new WPU(this, "wpua", m_porta, 0x37);
    tmr0.set_cpu(this, m_porta, 4, option_reg);
    tmr0.start(0);

    m_portc = new PicPortRegister(this,"portc", 8,0x3f);
    m_trisc = new PicTrisRegister(this,"trisc", m_portc, false);
}

P16F684::~P16F684()
{
    unassignMCLRPin();

    delete_file_registers(0x20, 0x7f);
    delete_file_registers(0xa0, 0xbf);

    remove_SfrReg(&tmr0);
    remove_SfrReg(&intcon_reg);
    remove_SfrReg(pir1);
    remove_SfrReg(&tmr1l);
    remove_SfrReg(&tmr1h);
    remove_SfrReg(&t1con);
    remove_SfrReg(&tmr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&pwm1con);
    remove_SfrReg(&eccpas);
    remove_SfrReg(&wdtcon);
    remove_SfrReg(&comparator.cmcon);
    remove_SfrReg(&comparator.cmcon1);
    remove_SfrReg(&adresh);
    remove_SfrReg(&adcon0);
    remove_SfrReg(&pie1);
    remove_SfrReg(&pcon);
    remove_SfrReg(osccon);
    remove_SfrReg(&osctune);
    remove_SfrReg(&ansel);
    remove_SfrReg(&pr2);
    remove_SfrReg(&comparator.vrcon);
    remove_SfrReg(get_eeprom()->get_reg_eedata());
    remove_SfrReg(get_eeprom()->get_reg_eeadr());
    remove_SfrReg(get_eeprom()->get_reg_eecon1());
    remove_SfrReg(get_eeprom()->get_reg_eecon2());
    remove_SfrReg(&adresl);
    remove_SfrReg(&adcon1);

    delete_SfrReg(m_portc);
    delete_SfrReg(m_trisc);

    delete_SfrReg(m_porta);
    delete_SfrReg(m_trisa);
    delete_SfrReg(m_ioca);
    delete_SfrReg(m_wpua);
    delete_SfrReg(pir1_3_reg);
    delete e;
}

void P16F684::create_iopin_map(void)
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

Processor * P16F684::construct(const char *name)
{
    P16F684 *p = new P16F684(name);

    p->create(256);
    p->create_invalid_registers ();

    return p;
}

void P16F684::create(int eesize)
{
    create_iopin_map();

    _14bit_processor::create();

    osccon = new OSCCON(this, "osccon" );

    e = new EEPROM_WIDE(this,pir1);
    e->initialize(eesize);
    e->set_intcon(&intcon_reg);
    set_eeprom_wide(e);

    status->rp_mask = 0x60;  // rp0 and rp1 are valid.
    indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
    indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

    P16F684::create_sfr_map();
}

void P16F684::create_sfr_map()
{
    pir_set_def.set_pir1(pir1);

    add_file_registers(0x20, 0x7f, 0);
    add_file_registers(0xa0, 0xbf, 0);
    alias_file_registers(0x70, 0x7f, 0x80);

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
    add_SfrReg(&intcon_reg, 0x00b, RegisterValue(0,0));

    alias_file_registers(0x0a,0x0b,0x80);
    add_SfrReg(pir1, 0x0c, RegisterValue(0,0));
    add_SfrReg(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
    add_SfrReg(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
    add_SfrReg(&t1con, 0x10, RegisterValue(0,0));
    add_SfrReg(&tmr2, 0x11, RegisterValue(0,0));
    add_SfrReg(&t2con, 0x12, RegisterValue(0,0));
    add_SfrReg(&ccpr1l, 0x13, RegisterValue(0,0));
    add_SfrReg(&ccpr1h, 0x14, RegisterValue(0,0));
    add_SfrReg(&ccp1con, 0x15, RegisterValue(0,0));
    add_SfrReg(&pwm1con, 0x16, RegisterValue(0,0));
    add_SfrReg(&eccpas, 0x17, RegisterValue(0,0));
    add_SfrReg(&wdtcon, 0x18, RegisterValue(0x08,0));
    add_SfrReg(&comparator.cmcon, 0x19, RegisterValue(0,0), "cmcon0");
    add_SfrReg(&comparator.cmcon1, 0x1a, RegisterValue(0,0), "cmcon1");
    add_SfrReg(&adresh,  0x1e, RegisterValue(0,0));
    add_SfrReg(&adcon0, 0x1f, RegisterValue(0,0));

    add_SfrReg(&pie1,   0x8c, RegisterValue(0,0));
    add_SfrReg(&pcon, 0x8e, RegisterValue(0,0));
    add_SfrReg(osccon, 0x8f, RegisterValue(0x60,0));

    add_SfrReg(&osctune, 0x90, RegisterValue(0,0),"osctune");
    add_SfrReg(&ansel, 0x91, RegisterValue(0xff,0));
    add_SfrReg(&pr2,    0x92, RegisterValue(0xff,0));
    add_SfrReg(m_wpua, 0x95, RegisterValue(0x37,0),"wpua");
    add_SfrReg(m_ioca, 0x96, RegisterValue(0,0),"ioca");
    add_SfrReg(&comparator.vrcon, 0x99, RegisterValue(0,0),"vrcon");
    add_SfrReg(get_eeprom()->get_reg_eedata(),  0x9a);
    add_SfrReg(get_eeprom()->get_reg_eeadr(),   0x9b);
    add_SfrReg(get_eeprom()->get_reg_eecon1(),  0x9c, RegisterValue(0,0));
    add_SfrReg(get_eeprom()->get_reg_eecon2(),  0x9d);
    add_SfrReg(&adresl,  0x9e, RegisterValue(0,0));
    add_SfrReg(&adcon1, 0x9f, RegisterValue(0,0));

    ansel.setAdcon1(&adcon1);
    ansel.setValidBits(0xff);

    // Link the comparator and voltage ref to porta
    comparator.initialize(&pir_set_def, NULL,
                          &(*m_porta)[0], &(*m_porta)[1],          // AN0 AN1
            0, 0,
            &(*m_porta)[2], &(*m_portc)[4]);        //OUT0 OUT1

    comparator.cmcon.setINpin(2, &(*m_portc)[0], "an4"); //AN4
    comparator.cmcon.setINpin(3, &(*m_portc)[1], "an5"); //AN5

    comparator.cmcon.set_tmrl(&tmr1l);
    comparator.cmcon1.set_tmrl(&tmr1l);

    comparator.cmcon.set_configuration(1, 0, AN0, AN1, AN0, AN1, ZERO);
    comparator.cmcon.set_configuration(2, 0, AN2, AN3, AN2, AN3, ZERO);
    comparator.cmcon.set_configuration(1, 1, AN1, AN2, AN0, AN2, NO_OUT);
    comparator.cmcon.set_configuration(2, 1, AN3, AN2, AN3, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 2, AN1, VREF, AN0, VREF, NO_OUT);
    comparator.cmcon.set_configuration(2, 2, AN3, VREF, AN2, VREF, NO_OUT);
    comparator.cmcon.set_configuration(1, 3, AN1, AN2, AN1, AN2, NO_OUT);
    comparator.cmcon.set_configuration(2, 3, AN3, AN2, AN3, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 4, AN1, AN0, AN1, AN0, NO_OUT);
    comparator.cmcon.set_configuration(2, 4, AN3, AN2, AN3, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 5, AN3, AN2, AN3, AN2, NO_OUT);
    comparator.cmcon.set_configuration(1, 6, AN1, AN2, AN1, AN2, OUT0);
    comparator.cmcon.set_configuration(2, 6, AN3, AN2, AN3, AN2, OUT1);
    comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
    comparator.vrcon.setValidBits(0xaf);

    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(&intcon_reg);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(7);
    adcon0.setChannel_shift(2);

    adcon1.setAdcon0(&adcon0);        // VCFG0, VCFG1 in adcon0
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

    if (pir1)
    {
        pir1->set_intcon(&intcon_reg);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);

    t2con.tmr2  = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
    pr2.tmr2    = &tmr2;

    eccpas.setIOpin(0, 0, &(*m_portc)[5]);
    eccpas.link_registers(&pwm1con, &ccp1con);

    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    ccp1con.setBitMask(0xff);
    ccp1con.pstrcon = &pstrcon;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;

    osccon->set_osctune(&osctune);
    osctune.set_osccon(osccon);
}

void P16F684::option_new_bits_6_7(uint bits)
{
    m_wpua->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
    m_porta->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}

void P16F684::create_config_memory()
{
    m_configMemory = new ConfigMemory(this,1);
    m_configMemory->addConfigWord(0,new ConfigF631((P16F631*)this));
    wdt.initialize(true); // default WDT enabled
    wdt.set_timeout(0.000035);
    set_config_word(0x2007, 0x3fff);
}

bool P16F684::set_config_word(uint address, uint cfg_word)
{
    enum {
        CFG_FOSC0 = 1<<0,
        CFG_FOSC1 = 1<<1,
        CFG_FOSC2 = 1<<2,
        CFG_WDTE  = 1<<3,
        CFG_MCLRE = 1<<5,
        CFG_IESO  = 1<<11,
    };

    if(address == config_word_address())
    {
        config_clock_mode = (cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2));
        if (osccon)
        {
            osccon->set_config_xosc(config_clock_mode < 3);
            osccon->set_config_irc(config_clock_mode == 4 || config_clock_mode == 5);
            osccon->set_config_ieso(cfg_word & CFG_IESO);
        }
        uint valid_pins = m_porta->getEnableMask();

        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE) assignMCLRPin(4);
        else                                     unassignMCLRPin();

        wdt.initialize((cfg_word & CFG_WDTE) == CFG_WDTE);

        set_int_osc(false);

        // AnalogReq is used so ADC does not change clock names
        // set_config_word is first called with default and then
        // often called a second time. the following call is to
        // reset porta so next call to AnalogReq sill set the pin name
        //
        (&(*m_porta)[4])->AnalogReq((Register *)this, false, "porta4");
        valid_pins |= 0x20;
        switch(config_clock_mode)
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
            m_trisa->setEnableMask(valid_pins);
        }
        return(true);
    }
    return false;
}
//======================================================================== 

Processor* P16F677::construct( const char *name )
{
    P16F677* p = new P16F677( name );

    p->create(256);
    p->set_hasSSP();
    p->create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F677::P16F677(const char *_name )
    : P16F631(_name ),
      ssp(this),
      anselh(this,"anselh" ),
      adresh(this,"adresh" ),
      adresl(this,"adresl" )
{
}

P16F677::~P16F677()
{
    delete_file_registers(0x20,0x3f);
    delete_file_registers(0xa0,0xbf);

    remove_SfrReg(&anselh);

    if (hasSSP())
    {
        remove_SfrReg(&ssp.sspbuf);
        remove_SfrReg(&ssp.sspcon);
        remove_SfrReg(&ssp.sspadd);
        remove_SfrReg(&ssp.sspstat);
    }
    remove_SfrReg(&adresl);
    remove_SfrReg(&adresh);
    remove_SfrReg(&adcon0);
    remove_SfrReg(&adcon1);
    //delete m_cvref;
    //delete m_v06ref;
}

void P16F677::create_sfr_map()
{
    ansel.setAdcon1(&adcon1);
    ansel.setAnselh(&anselh);
    anselh.setAdcon1(&adcon1);
    anselh.setAnsel(&ansel);
    anselh.setValidBits(0x0f);
    ansel.setValidBits(0xff);
    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(&intcon_reg);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0xf);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);
    adcon0.setValidBits(0xff);

    adcon1.setValidBits(0xb0);
    adcon1.setAdcon0(&adcon0);
    adcon1.setNumberOfChannels(14);
    adcon1.setValidCfgBits(ADCON1::VCFG0 , 6);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);

    adcon1.setIOPin(8, &(*m_portc)[6]);
    adcon1.setIOPin(9, &(*m_portc)[7]);
    adcon1.setIOPin(10, &(*m_portb)[4]);
    adcon1.setIOPin(11, &(*m_portb)[5]);
    adcon1.setVoltRef(12, 0.0);
    adcon1.setVoltRef(13, 0.0);
    adcon1.setVrefHiConfiguration(2, 1); // set a2d modes where an1 is Vref+

    add_SfrReg(&anselh, 0x11f, RegisterValue(0x0f,0));
    add_file_registers(0x20,0x3f,0);
    add_file_registers(0xa0,0xbf,0);

    if( hasSSP() )
    {
        add_SfrReg(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
        add_SfrReg(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
        add_SfrReg(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
        add_SfrReg(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");

        ssp.initialize(
                    get_pir_set(),    // PIR
                    &(*m_portb)[6],   // SCK
                &(*m_portc)[6],   // SS
                &(*m_portc)[7],   // SDO
                &(*m_portb)[4],    // SDI
                m_trisb,          // i2c tris port
                SSP_TYPE_SSP
                );
    }
    add_SfrReg(&adresl, 0x9e, RegisterValue(0,0));
    add_SfrReg(&adresh, 0x1e, RegisterValue(0,0));
    add_SfrReg(&adcon0, 0x1f, RegisterValue(0,0));
    add_SfrReg(&adcon1, 0x9f, RegisterValue(0,0));
}
//========================================================================
//
// Pic 16F685 
//

Processor* P16F685::construct(const char *name)
{
    P16F685 *p = new P16F685(name);

    p->create(256);
    p->create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F685::P16F685(const char *_name )
    : P16F677(_name ),
      t2con(this, "t2con" ),
      pr2(this, "pr2" ),
      tmr2(this, "tmr2" ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      ccp1con(this, "ccp1con" ),
      ccpr1l(this, "ccpr1l" ),
      ccpr1h(this, "ccpr1h" ),
      pcon(this, "pcon" ),
      eccpas(this, "eccpas" ),
      pwm1con(this, "pwm1con" ),
      pstrcon(this, "pstrcon" )
{
    set_hasSSP();
}

P16F685::~P16F685()
{
    delete_file_registers(0xc0,0xef);
    delete_file_registers(0x120,0x16f);
    remove_SfrReg(&pstrcon);
    remove_SfrReg(&tmr2);
    remove_SfrReg(&t2con);
    remove_SfrReg(&pr2);
    remove_SfrReg(&ccpr1l);
    remove_SfrReg(&ccpr1h);
    remove_SfrReg(&ccp1con);
    remove_SfrReg(&pwm1con);
    remove_SfrReg(&eccpas);
}

void P16F685::create_sfr_map()
{
    P16F677::create_sfr_map();

    add_SfrReg(get_eeprom()->get_reg_eedatah(),  0x10e );
    add_SfrReg(get_eeprom()->get_reg_eeadrh(),   0x10f);

    // Enable program memory reads and writes.
    get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);


    add_SfrReg(&tmr2,   0x11, RegisterValue(0,0));
    add_SfrReg(&t2con,  0x12, RegisterValue(0,0));
    add_SfrReg(&pr2,    0x92, RegisterValue(0xff,0));
    t2con.tmr2  = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
    pr2.tmr2    = &tmr2;

    eccpas.setIOpin(0, 0, &(*m_portb)[0]);
    eccpas.link_registers(&pwm1con, &ccp1con);
    add_SfrReg(&pstrcon, 0x19d, RegisterValue(1,0));

    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    ccp1con.setBitMask(0xff);
    ccp1con.pstrcon = &pstrcon;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;

    add_SfrReg(&ccpr1l, 0x15, RegisterValue(0,0));
    add_SfrReg(&ccpr1h, 0x16, RegisterValue(0,0));
    add_SfrReg(&ccp1con, 0x17, RegisterValue(0,0));

    add_SfrReg(&pwm1con, 0x1c, RegisterValue(0,0));
    add_SfrReg(&eccpas, 0x1d, RegisterValue(0,0));
    //  add_file_registers(0x20,0x3f,0);
    //  add_file_registers(0xa0,0xef,0);
    add_file_registers(0xc0,0xef,0);
    add_file_registers(0x120,0x16f,0);


}
//========================================================================
//
// Pic 16F687 
//

Processor * P16F687::construct(const char *name)
{
    P16F687 *p = new P16F687(name);

    p->create(256);
    p->create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F687::P16F687(const char *_name )
    : P16F677(_name ),
      tmr1l(this, "tmr1l" ),
      tmr1h(this, "tmr1h" ),
      pcon(this, "pcon" ),
      usart(this)
{
    set_hasSSP();
}

P16F687::~P16F687()
{
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    remove_SfrReg(&usart.spbrgh);
    remove_SfrReg(&usart.baudcon);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);
}

void P16F687::create_sfr_map()
{
    P16F677::create_sfr_map();

    add_SfrReg(get_eeprom()->get_reg_eedatah(),  0x10e);
    add_SfrReg(get_eeprom()->get_reg_eeadrh(),   0x10f);

    //  add_file_registers(0x20,0x3f,0);
    //  add_file_registers(0xa0,0xbf,0);

    usart.initialize(pir1,&(*m_portb)[7], &(*m_portb)[5],
            new _TXREG(this,"txreg", &usart),
            new _RCREG(this,"rcreg", &usart));

    add_SfrReg(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
    add_SfrReg(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg(&usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
    add_SfrReg(&usart.baudcon,  0x9b,RegisterValue(0x40,0),"baudctl");
    add_SfrReg(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
    add_SfrReg(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
    usart.set_eusart(true);

}
//========================================================================
//
// Pic 16F689 
//

Processor * P16F689::construct(const char *name)
{
    P16F689 *p = new P16F689(name);

    p->create(256);
    p->create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F689::P16F689(const char *_name )
    : P16F687(_name )
{
    set_hasSSP();
}

//========================================================================
//
Processor * P16F690::construct(const char *name)
{
    P16F690 *p = new P16F690(name);

    p->create(256);
    p->create_sfr_map();
    p->create_invalid_registers ();

    return p;
}

P16F690::P16F690(const char *_name )
    : P16F685(_name ),
      ccp2con(this, "ccp2con" ),
      ccpr2l(this, "ccpr2l" ),
      ccpr2h(this, "ccpr2h" ),
      usart(this)
{
    set_hasSSP();
}

P16F690::~P16F690()
{
    remove_SfrReg(&usart.rcsta);
    remove_SfrReg(&usart.txsta);
    remove_SfrReg(&usart.spbrg);
    remove_SfrReg(&usart.spbrgh);
    remove_SfrReg(&usart.baudcon);
    delete_SfrReg(usart.txreg);
    delete_SfrReg(usart.rcreg);
}

void P16F690::create_sfr_map()
{
    P16F685::create_sfr_map();

    tmr2.ssp_module[0] = &ssp;
    eccpas.setIOpin(0, 0, &(*m_portb)[0]);
    eccpas.link_registers( &pwm1con, &ccp1con );

    usart.initialize( pir1,&(*m_portb)[7], &(*m_portb)[5],
            new _TXREG( this,"txreg", &usart ),
            new _RCREG( this,"rcreg", &usart ));

    add_SfrReg( &usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
    add_SfrReg( &usart.txsta, 0x98, RegisterValue(2,0),"txsta");
    add_SfrReg( &usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
    add_SfrReg( &usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
    add_SfrReg( &usart.baudcon,  0x9b,RegisterValue(0x40,0),"baudctl");
    add_SfrReg( usart.txreg,  0x19, RegisterValue(0,0),"txreg");
    add_SfrReg( usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
    usart.set_eusart(true);
}
