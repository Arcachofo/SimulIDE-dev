/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include <QDomDocument>
#include <QFileInfo>
#include <QObject>

#include "mcucreator.h"
#include "e_mcu.h"
#include "mcu.h"
#include "mcuport.h"
#include "mcupin.h"
#include "mcutimer.h"
//#include "mcuinterrupts.h"
#include "i51interrupt.h"

// Cores
#include "avrcore.h"

#include "i51core.h"
#include "i51timer.h"

#include "utils.h"

QString McuCreator::m_core = "";

McuCreator::McuCreator()
{

}

McuCreator::~McuCreator(){}

int McuCreator::createMcu( Mcu* mcu )
{
    QString dataFile = mcu->m_dataFile;

    QFile file( dataFile );
    if( !file.open( QFile::ReadOnly | QFile::Text) )
    {
        MessageBoxNB( "Mcu::create",
                  QObject::tr( "Cannot read file:\n%1:\n%2." ).arg(dataFile).arg(file.errorString()) );
          return 1;
    }

    QDomDocument domDoc;
    if( !domDoc.setContent(&file) )
    {
         MessageBoxNB( "Mcu::create",
                   QObject::tr( "Cannot set file:\n%1\nto DomDocument" ) .arg(dataFile));
         file.close();
         return 2;
    }
    file.close();

    QDomElement root = domDoc.documentElement();

    m_core = root.attribute( "core" );
    createDataMem( root.attribute( "data" ).toUInt(0,0), mcu );
    createProgMem( root.attribute( "prog" ).toUInt(0,0), mcu );
    //createEeprom( root.attribute( "eeprom" ).toUInt(0,0) );
    mcu->m_wordSize = root.attribute( "progword" ).toUInt(0,0);
    mcu->m_instCycle = root.attribute( "inst_cycle" ).toDouble();

    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if     ( el.tagName() == "registers" )  createRegisters( &el, mcu );
        else if( el.tagName() == "interrupts" ) createInterrupts( &el, mcu );
        else if( el.tagName() == "port" )       createPort( &el, mcu );
        else if( el.tagName() == "timer" )      createTimer( &el, mcu );

        node = node.nextSibling();
    }

    createCore( m_core, mcu );

    QDomElement el = root.firstChild().toElement();
    createStack( &el, mcu );

    return 0;
}

void McuCreator::createProgMem( uint32_t size, Mcu* mcu )
{
    mcu->m_progMemSize = size;
    mcu->m_progMem.resize( size, 0xFFFF );
}

void McuCreator::createDataMem( uint32_t size, Mcu* mcu )
{
    mcu->m_dataMemSize = size;
    mcu->m_dataMem.resize( size, 0 );
}

void McuCreator::createRegisters( QDomElement* e, Mcu* mcu )
{
    mcu->m_regStart = e->attribute("start").toUInt(0,0);
    mcu->m_regEnd   = e->attribute("end").toUInt(0,0);

    QDomNode node = e->firstChild();
    while( !node.isNull() ) // Create Registers
    {
        QDomElement el = node.toElement();

        QString  regName = el.attribute("name");
        uint16_t regAddr = el.attribute("addr").toUInt(0,0)+mcu->m_regStart;
        uint8_t resetVal = el.attribute("reset").toUInt(0,0);

        eMcu::regInfo_t regInfo = { regAddr, resetVal };
        mcu->m_regInfo.insert( regName, regInfo );

        QString bits = el.attribute( "bits" );
        if( !bits.isEmpty() ) // Create bitMasks
        {
            if( bits == "0-7")
            {
                for( int i=0; i<8; ++i )
                {
                    QString bitName = regName+QString::number( i );
                    uint8_t mask = 1<<i;
                    mcu->m_bitMasks.insert( bitName, mask );
                    mcu->m_bitRegs.insert( bitName, regAddr );
                }
            }else
            {
                QStringList bitList = bits.split(",");
                for( int i=0; i< bitList.size(); ++i )
                {
                    QString bitName = bitList.at( i );
                    uint8_t mask = 1<<i;
                    mcu->m_bitMasks.insert( bitName, mask );
                    mcu->m_bitRegs.insert( bitName, regAddr );
                }
            }
        }
        node = node.nextSibling();
    }
    // CReate STATUS Reg
    uint16_t addr = mcu->m_regInfo.value( e->attribute( "status" ) ).address;
    mcu->m_sregAddr = addr;
    mcu->m_sreg.resize( 8, 0 );
    mcu->watchRegister( addr, R_WRITE, (eMcu*)mcu, &eMcu::writeStatus );
    mcu->watchRegister( addr, R_READ, (eMcu*)mcu, &eMcu::readStatus );
}

void McuCreator::createInterrupts( QDomElement* i, Mcu* mcu )
{
    QString enable = i->attribute("enable");
    mcu->watchBits( enable, R_WRITE, (eMcu*)mcu, &eMcu::enableInterrupts );

    QDomNode node = i->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        QString  intName    = el.attribute("name");
        uint16_t intVector  = el.attribute("vector").toUInt(0,0);

        Interrupt* iv = NULL;
        if( m_core == "8051" ) iv = I51Interrupt::getInterrupt( intName, intVector, (eMcu*)mcu );
        if( !iv ) return;

        Interrupt::m_interrupts.insert( intName, iv );

        enable = el.attribute("enable");
        mcu->watchBits( enable, R_WRITE, iv, &Interrupt::enableFlag );

        QString intFlag = el.attribute("flag");
        iv->m_flagMask = mcu->m_bitMasks.value( intFlag );
        iv->m_flagReg  = mcu->m_bitRegs.value( intFlag );

        QString intPrio = el.attribute("priority");
        bool ok = false;
        uint8_t prio = intPrio.toUInt(&ok,0);
        if( ok ) iv->setPriority( prio );
        else     mcu->watchBits( intPrio, R_WRITE, iv, &Interrupt::setPriority );

        if( el.hasAttribute("mode") )
        {
            QString mode = el.attribute("mode");
            mcu->watchBits( mode, R_WRITE, iv, &Interrupt::setMode );
        }

        node = node.nextSibling();
    }
}

void McuCreator::createPort( QDomElement* p, Mcu* mcu )
{
    //Create PORT
    McuPort* port = new McuPort( mcu );
    port->m_name = p->attribute( "name" );
    McuPort::m_ports.insert( port->name(), port );

    // Create PORT Pins
    port->m_numPins = p->attribute( "pins" ).toUInt(0,0);
    port->m_pins.resize( port->m_numPins );
    for( int i=0; i<port->m_numPins; ++i )
    {
        McuPin* pin = new McuPin( port, i, port->m_name+QString::number(i), mcu );
        port->m_pins[i] = pin;
    }
    // Connect to PORT Out Register
    uint16_t   addr = mcu->getRegAddress( p->attribute( "outreg" ) );
    port->m_outAddr = addr;
    port->m_outReg  = mcu->m_dataMem.data()+addr;
    mcu->watchRegister( addr, R_WRITE, port, &McuPort::outChanged );

    // Connect to PORT In Register
    QString inreg = p->attribute( "inreg" );
    if( !inreg.isEmpty() )
    {
        addr = mcu->getRegAddress( inreg );
        port->m_inAddr = addr;
        port->m_inReg  = mcu->m_dataMem.data()+port->m_inAddr;
        mcu->watchRegister( addr, R_READ, port, &McuPort::readInReg );
    }
    // Connect to PORT Dir Register
    QString dirreg = p->attribute( "dirreg" );
    if( !dirreg.isEmpty() )
    {
        addr = mcu->getRegAddress( dirreg );
        port->m_dirAddr = addr;
        port->m_dirReg  = mcu->m_dataMem.data()+addr;
        mcu->watchRegister( addr, R_WRITE, port, &McuPort::dirChanged );
    }
    // Permanent Directions
    if( p->hasAttribute("dirmask") )
    {
        uint8_t dirMask = p->attribute("dirmask").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            port->m_pins[i]->m_dirMask = (dirMask & 1<<i);
    }
    // Permanent Pullups
    if( p->hasAttribute("pullups") )
    {
        uint8_t pullup = p->attribute("pullups").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            port->m_pins[i]->m_puMask = (pullup & 1<<i);
    }
    // OPen Drain
    if( p->hasAttribute("opencol") )
    {
        uint8_t opencol = p->attribute("opencol").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            port->m_pins[i]->m_openColl = (opencol & 1<<i);
    }
    // Interrupts and...
    QDomNode node = p->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "raiseint" )  // Interrupts
        {
            QString intName = el.attribute("intname");
            Interrupt* inte = Interrupt::m_interrupts.value( intName );

            QString source = el.attribute("source");
            mcu->watchBits( source, R_WRITE, inte, &Interrupt::raise );
        }
        node = node.nextSibling();
    }
}

void McuCreator::createTimer( QDomElement* t, Mcu* mcu )
{
    McuTimer* timer = NULL;
    if( m_core == "8051" ) timer = new I51Timer( mcu );
    if( !timer ) return;

    timer->m_name = t->attribute("name");
    McuTimer::m_timers.insert( timer->name(), timer );

    if( t->hasAttribute("counter") )
    {
        QString counter = t->attribute("counter");
        QString lowByte = counter;
        QString highByte ="";

        if( counter.contains(",") )
        {
            QStringList regs = counter.split(",");

            lowByte = regs.takeFirst();
            highByte = regs.takeFirst();
        }
        if( !lowByte.isEmpty() )
        {
            timer->m_countL = mcu->getReg( lowByte );
            mcu->watchRegsName( lowByte, R_WRITE, timer, &McuTimer::countWriteL );
            mcu->watchRegsName( lowByte, R_READ,  timer, &McuTimer::countReadL );
        }
        if( !highByte.isEmpty() )
        {
            timer->m_countH = mcu->getReg( highByte );
            mcu->watchRegsName( highByte, R_WRITE, timer, &McuTimer::countWriteH );
            mcu->watchRegsName( highByte, R_READ,  timer, &McuTimer::countReadH );
        }
    }

    //timer->m_nBits = t->attribute( "bits" ).toUInt(0,0);
    if( t->hasAttribute("enable") )
    {
        QString enable = t->attribute("enable");
        mcu->watchBits( enable, R_WRITE, timer, &McuTimer::enable );
    }
    if( t->hasAttribute("mode") )
    {
        QString mode = t->attribute("mode");
        mcu->watchBits( mode, R_WRITE, timer, &McuTimer::configure );
    }

    QDomNode node = t->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "raiseint" )
        {
            QString intName = el.attribute("intname");
            Interrupt* inte = Interrupt::m_interrupts.value( intName );

            QString source  = el.attribute("source");
            if( source == "OVERFLOW" ) timer->on_tov.connect( inte, &Interrupt::raise );
            if( source == "COMP"     ) timer->on_comp.connect( inte, &Interrupt::raise );
        }
        if( el.tagName() == "extclock" )
        {
            /// TODO
        }
        node = node.nextSibling();
    }
}

void McuCreator::createCore( QString core, Mcu* mcu )
{
    if( core == "AVR" )  mcu->cpu = new AvrCore( mcu );
    if( core == "8051" ) mcu->cpu = new I51Core( mcu );
}

void McuCreator::createStack( QDomElement* s, Mcu* mcu )
{
    if( !(s->tagName() == "stack") ) return;

    QString inc = s->attribute("increment");

    mcu->cpu->m_spPre = inc.contains("pre");
    mcu->cpu->m_spInc = ((inc.contains("inc"))?  1:-1);
}
