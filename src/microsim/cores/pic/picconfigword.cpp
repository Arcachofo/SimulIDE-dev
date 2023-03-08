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

PicConfigWord::PicConfigWord( eMcu* mcu, QString name )
             : ConfigWord( mcu, name )
{
}
PicConfigWord::~PicConfigWord(){}

ConfigWord* PicConfigWord::createCfgWord( eMcu* mcu, QString name, QString type )
{
    if( type == "00" ) return new PicConfigWord00( mcu, name );
    return NULL;
}

//-----------------------------

PicConfigWord00::PicConfigWord00( eMcu* mcu, QString name )
               : PicConfigWord( mcu, name )
               , eElement( name )
{
}
PicConfigWord00::~PicConfigWord00(){}

bool PicConfigWord00::setCfgWord( uint16_t addr, uint16_t data )
{
    bool ok = ConfigWord::setCfgWord( addr, data );
    if( !ok ) return false;

    bool ioI = false; // m_CLKIN is IO?
    bool ioO = false; // m_CLKOUT is IO?
    bool clO = false; // m_CLKOUT outputs clock signal?

    if( addr == 0x2007 )
    {
        QString msg = "      ";
        switch( data & 7 ) {
            case 0:                   msg +="LP osc: Crystal RA6 and RA7"; break; // Low-power crystal on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 1:                   msg +="XT osc: Crystal RA6 and RA7"; break; // Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 2:                   msg +="HS osc: Crystal RA6 and RA7"; break; // High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN
            case 3: ioO = true;       msg +="EC: I/O RA6, CLKIN RA7     "; break; // I/O function on RA6/OSC2/CLKOUT, CLKIN on RA7/OSC1/CLKIN
            case 4: ioO = ioI = true; msg +="INTOSCIO: I/O RA6 and RA7  "; break; // INTOSCIO osc: I/O function on RA6/OSC2/CLKOUT and on RA7/OSC1/CLKIN
            case 5: clO = ioI = true; msg +="INTOSC: CLKOUT RA6, I/O RA7"; break; // CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN
            case 6: ioO = true;       msg +="RCIO: I/O RA6, RC RA7      "; break; // I/O function on RA6/OSC2/CLKOUT, RC on RA7/OSC1/CLKIN
            case 7: clO = true;       msg +="RC: CLKOUT RA6, RC RA7     "; break; // CLKOUT function on RA6/OSC2/CLKOUT, RC on RA7/OSC1/CLKIN
        }
        qDebug()<<msg;
        if( m_mcu->intOsc() ) m_mcu->intOsc()->configPins( ioI, ioO, clO );
        else qDebug() <<"PicConfigWord00::setCfgWord Error: IntOsc does not exist";
        m_mcu->component()->removeProperty("Ext_Osc");

        bool wdte = data & 1<<3;
        if( wdte ) m_mcu->component()->enableWdt( true );
        m_mcu->component()->removeProperty("Wdt_enabled");
        msg = wdte ? "Enabled":"Disabled";
        qDebug()<<"      Watchdog:"<< msg;

        bool mclr = data & 1<<5;
        if( mclr ) m_mcu->component()->enableRstPin( true );
        m_mcu->component()->removeProperty("Rst_enabled");
        msg = mclr ? "Enabled":"Disabled";
        qDebug()<<"      MCLR:    "<< msg;
        /// m_mcu->component()->remPropGroup( tr("Config") );
    }
    qDebug()<<"";
    return true;
}
