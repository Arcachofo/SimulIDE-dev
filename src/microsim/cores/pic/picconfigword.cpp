/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "picconfigword.h"
#include "picintosc.h"
#include "simulator.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcu.h"


ConfigWord* PicConfigWord::createCfgWord( eMcu* mcu, QString name, QString type ) // Static
{
    if     ( type == "00" ) return new PicConfigWord00( mcu, name ); // 16F88x
    else if( type == "01" ) return new PicConfigWord01( mcu, name ); // 16F62x
    else if( type == "02" ) return new PicConfigWord02( mcu, name ); // 16F87x
    else if( type == "03" ) return new PicConfigWord03( mcu, name ); // 10F20x
    return NULL;
}

PicConfigWord::PicConfigWord( eMcu* mcu, QString name )
             : ConfigWord( mcu, name )
{
}
PicConfigWord::~PicConfigWord(){}

void PicConfigWord::configClock( uint8_t fosc, bool wdte, bool mclr )
{
    bool ioI = false; // m_CLKIN is IO?
    bool ioO = false; // m_CLKOUT is IO?
    bool clO = false; // m_CLKOUT outputs clock signal?

    QString msg = "      ";
    switch( fosc ) {
        case 0:                   msg +="LP osc:   Crystal OSC2 and OSC1"; break; // Low-power crystal on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 1:                   msg +="XT osc:   Crystal OSC2 and OSC1"; break; // Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 2:                   msg +="HS osc:   Crystal OSC2 and OSC1"; break; // High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 3: ioO = true;       msg +="EC:       I/O OSC2, CLKIN OSC1 "; break; // I/O function on RA6/OSC2/CLKOUT, CLKIN on RA7/OSC1/CLKIN
        case 4: ioO = ioI = true; msg +="INTOSCIO: I/O OSC2 and OSC1    "; break; // INTOSCIO osc: I/O function on RA6/OSC2/CLKOUT and on RA7/OSC1/CLKIN
        case 5: clO = ioI = true; msg +="INTOSC:   CLKOUT OSC2, I/O OSC1"; break; // CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN
        case 6: ioO = true;       msg +="RCIO:     I/O OSC2, RC OSC1    "; break; // I/O function on RA6/OSC2/CLKOUT, RC on RA7/OSC1/CLKIN
        case 7: clO = true;       msg +="RC:       CLKOUT OSC2, RC OSC1 "; break; // CLKOUT function on RA6/OSC2/CLKOUT, RC on RA7/OSC1/CLKIN
    }
    qDebug()<<msg;
    if( m_mcu->intOsc() ) m_mcu->intOsc()->configPins( ioI, ioO, clO );
    else qDebug() <<"PicConfigWord::setCfgWord Error: IntOsc does not exist";

    if( wdte ) m_mcu->component()->enableWdt( true );
    msg = wdte ? "Enabled":"Disabled";
    qDebug()<<"      Watchdog:"<< msg;

    if( mclr ) m_mcu->component()->enableRstPin( true );
    msg = mclr ? "Enabled":"Disabled";
    qDebug()<<"      MCLR:    "<< msg;
}

//-----------------------------

PicConfigWord00::PicConfigWord00( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
{
}
PicConfigWord00::~PicConfigWord00(){}

bool PicConfigWord00::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    if( addr == 0x2007 )
    {
        uint8_t fosc = data & 7;
        bool wdte = data & 1<<3;
        bool mclr = data & 1<<5;
        configClock( fosc, wdte, mclr );
    }
    qDebug()<<"";
    return true;
}

//-----------------------------

PicConfigWord01::PicConfigWord01( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
{
}
PicConfigWord01::~PicConfigWord01(){}

bool PicConfigWord01::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    if( addr == 0x2007 )
    {
        uint8_t fosc = (data & 3) | (data & 1<<4)>>2;
        bool wdte = data & 1<<2;
        bool mclr = data & 1<<5;
        configClock( fosc, wdte, mclr );
    }
    qDebug()<<"";
    return true;
}

//-----------------------------

PicConfigWord02::PicConfigWord02( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
{
}
PicConfigWord02::~PicConfigWord02(){}

bool PicConfigWord02::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    bool clO = false; // m_CLKOUT outputs clock signal?

    if( addr == 0x2007 )
    {
        QString msg = "      ";
        switch( data & 3 ) {
        case 0:             msg +="LP osc:   Crystal OSC2 and OSC1"; break; // Low-power crystal on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 1:             msg +="XT osc:   Crystal OSC2 and OSC1"; break; // Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 2:             msg +="HS osc:   Crystal OSC2 and OSC1"; break; // High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
        case 3: clO = true; msg +="RC osc:   CLKOUT OSC2, RC OSC1 "; break; // CLKOUT function on RA6/OSC2/CLKOUT, RC on RA7/OSC1/CLKIN
        }
        qDebug()<<msg;

        if( m_mcu->intOsc() ) m_mcu->intOsc()->configPins( false, clO, clO );
        else qDebug() <<"PicConfigWord02::setCfgWord Error: IntOsc does not exist";

        bool wdte = data & 1<<2;
        if( wdte ) m_mcu->component()->enableWdt( true );
        msg = wdte ? "Enabled":"Disabled";
        qDebug()<<"      Watchdog:"<< msg;
    }
    qDebug()<<"";
    return true;
}

//-----------------------------

PicConfigWord03::PicConfigWord03( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
{
}
PicConfigWord03::~PicConfigWord03(){}

bool PicConfigWord03::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    if( addr == 0x2007 )
    {
        bool wdte = data & 1<<2;
        if( wdte ) m_mcu->component()->enableWdt( true );
        QString msg = wdte ? "Enabled":"Disabled";
        qDebug()<<"      Watchdog:"<< msg;

        bool mclr = data & 1<<4;
        if( mclr ) m_mcu->component()->enableRstPin( true );
        msg = mclr ? "Enabled":"Disabled";
        qDebug()<<"      MCLR:    "<< msg;
    }
    qDebug()<<"";
    return true;
}

//-----------------------------

PicConfigWord04::PicConfigWord04( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
{
}
PicConfigWord04::~PicConfigWord04(){}

bool PicConfigWord04::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    if( addr == 0x8007 )
    {
        bool clO = (data & 1<<11) == 0; // m_CLKOUT outputs clock signal? CLKOUTEN
        bool ioO = !clO;  // m_CLKOUT is IO?
        bool ioI = false; // m_CLKIN is IO?

        uint8_t fosc = data & 7;

        QString msg = "      ";
        switch( fosc ) {
            case 0: clO = ioO = false; msg +="LP osc:   Crystal OSC2 and OSC1"; break; // Low-power crystal on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 1: clO = ioO = false; msg +="XT osc:   Crystal OSC2 and OSC1"; break; // Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 2: clO = ioO = false; msg +="HS osc:   Crystal OSC2 and OSC1"; break; // High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 3:                    msg +="EC:       CLKIN OSC1           "; break; // CLKIN on RA7/OSC1/CLKIN
            case 4: ioI = true;        msg +="INTOSCIO: I/O OSC1             "; break; // INTOSCIO osc: I/O function on RA7/OSC1/CLKIN
            case 5:                    msg +="ECL:      Ext Clock on OSC1    "; break; // External Clock on RA7/OSC1/CLKIN
            case 6:                    msg +="ECM:      Ext Clock on OSC1    "; break; // External Clock on RA7/OSC1/CLKIN
            case 7:                    msg +="RC:       Ext Clock on OSC1    "; break; // External Clock on RA7/OSC1/CLKIN
        }
        qDebug()<<msg;
        if( m_mcu->intOsc() ) m_mcu->intOsc()->configPins( ioI, ioO, clO );
        else qDebug() <<"PicConfigWord04::setCfgWord Error: IntOsc does not exist";

        bool wdte = data & 1<<4;
        if( wdte ) m_mcu->component()->enableWdt( true );
        msg = wdte ? "Enabled":"Disabled";
        qDebug()<<"      Watchdog:"<< msg;

        bool mclr = data & 1<<6;
        if( mclr ) m_mcu->component()->enableRstPin( true );
        msg = mclr ? "Enabled":"Disabled";
        qDebug()<<"      MCLR:    "<< msg;
    }
    qDebug()<<"";
    return true;
}
