/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>
#include <QFileInfo>
#include <QDebug>
#include <QObject>
#include <math.h>

#include "mcucreator.h"
#include "datautils.h"
#include "regwatcher.h"
#include "e_mcu.h"
#include "mcu.h"
#include "mcuport.h"
#include "ioport.h"
//#include "mcuportctrl.h"
#include "mcupin.h"
#include "extmem.h"
#include "intmem.h"

#include "usarttx.h"
#include "usartrx.h"

// Cores
#include "avrcore.h"
#include "avrport.h"
#include "avrtimer.h"
#include "avrocunit.h"
#include "avrocm.h"
#include "avricunit.h"
#include "avrinterrupt.h"
#include "avrusart.h"
#include "avradc.h"
#include "avrtwi.h"
#include "avrspi.h"
#include "avrwdt.h"
#include "avreeprom.h"
#include "avrcomparator.h"
#include "avrsleep.h"

#include "pic14core.h"
#include "pic14ecore.h"
#include "picport.h"
#include "pictimer.h"
#include "picccpunit.h"
#include "picicunit.h"
#include "picinterrupt.h"
#include "pic14einterrupt.h"
#include "picusart.h"
#include "picmssp.h"
#include "pictwi.h"
#include "picspi.h"
#include "picadc.h"
#include "piccomparator.h"
#include "picvref.h"
#include "picwdt.h"
#include "piceeprom.h"
#include "picconfigword.h"
#include "picintosc.h"

#include "i51core.h"
#include "i51timer.h"
#include "i51interrupt.h"
#include "i51usart.h"
#include "i51port.h"

#include "mcs65core.h"
#include "intmemcore.h"

#include "z80core.h"

#include "scriptcpu.h"
#include "scriptport.h"
#include "scriptusart.h"
#include "scriptspi.h"
#include "scriptprop.h"

#include "utils.h"


QString McuCreator::m_core = "";
QString McuCreator::m_CompName = "";
QString McuCreator::m_basePath = "";
QString McuCreator::m_txRegName = "";
Mcu*    McuCreator::m_mcuComp = NULL;
eMcu*   McuCreator::mcu = NULL;
McuTwi* McuCreator::m_twi = NULL;
McuSpi* McuCreator::m_spi = NULL;
bool    McuCreator::m_newStack;
QDomElement McuCreator::m_stackEl;
std::vector<ScriptPerif*> McuCreator::m_scriptPerif;

McuCreator::McuCreator(){}
McuCreator::~McuCreator(){}

int McuCreator::createMcu( Mcu* mcuComp, QString name )
{
    m_CompName = name;
    m_mcuComp  = mcuComp;
    m_newStack = false;
    m_scriptPerif.clear();

    mcu = &(mcuComp->m_eMcu);
    QString dataFile = mcuComp->m_dataFile;
    m_basePath = QFileInfo( dataFile ).absolutePath();
    dataFile   = QFileInfo( dataFile ).fileName();

    int error = processFile( dataFile, true );

    if( error == 0 ) mcu->getRamTable()->setRegisters( mcu->m_regInfo.keys() );
    return error;
}

int McuCreator::processFile( QString fileName, bool main )
{
    fileName = m_basePath+"/"+fileName;
    QDomDocument domDoc = fileToDomDoc( fileName, "McuCreator::processFile" );
    if( domDoc.isNull() ) return 1;

    QDomElement root = domDoc.documentElement();

    if( root.hasAttribute("core") ) m_core = root.attribute("core");

    if( root.hasAttribute("progword") )   mcu->m_wordSize = root.attribute("progword").toUInt(0,0);
    if( root.hasAttribute("inst_cycle") ) mcu->m_cPerInst = root.attribute("inst_cycle").toDouble();
    if( root.hasAttribute("data") )       createDataMem( root.attribute("data").toUInt(0,0) );
    if( root.hasAttribute("prog") )       createProgMem( root.attribute("prog").toUInt(0,0) );
    if( root.hasAttribute("eeprom") )     createRomMem( root.attribute("eeprom").toUInt(0,0) );
    if( root.hasAttribute("freq") )       m_mcuComp->setFreq( root.attribute("freq").toDouble() );
    else if( m_core == "AVR")             m_mcuComp->setFreq( 16*1e6 );
    else if( m_core.startsWith("Pic") )   m_mcuComp->setFreq( 20*1e6 );

    int error = 0;
    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();
        QString   part = el.tagName();

        if     ( part == "regblock" )   createRegisters( &el );
        else if( part == "datablock" )  createDataBlock( &el );
        else if( part == "progblock" )  createProgBlock( &el );
        else if( part == "stack" )      { m_stackEl = el; m_newStack = true; }
        else if( part == "interrupts" ) createInterrupts( &el );
        else if( part == "port" )       createMcuPort( &el );
        else if( part == "mcuport" )    createMcuPort( &el );
        else if( part == "ioport" )     createIoPort( &el );
        else if( part == "timer" )      createTimer( &el );
        else if( part == "ocm" )        createOcm( &el );
        else if( part == "ccpunit" )    createCcpUnit( &el );
        else if( part == "msspunit" )   createMsspUnit( &el );
        else if( part == "usart" )      createUsart( &el );
        else if( part == "adc" )        createAdc( &el );
        else if( part == "comp" )       createAcomp( &el );
        else if( part == "vref" )       createVref( &el );
        else if( part == "twi" )        createTwi( &el );
        else if( part == "spi" )        createSpi( &el );
        else if( part == "wdt" )        createWdt( &el );
        else if( part == "rom" )        createEeprom( &el );
        else if( part == "sleep" )      createSleep( &el );
        else if( part == "configwords") createCfgWord( &el );
        else if( part == "intosc")      createIntOsc( &el );
        //else if( part == "extmem" )     createExtMem( &el );
        else if( part == "intmem" )     createIntMem( &el );

        else if( part == "include" )
        {
            error = processFile( el.attribute("file") );
            if( error ) return error;
        }
        node = node.nextSibling();
    }
    if( root.hasAttribute("core") )
    {
        if     ( m_core == "AVR" )      mcu->cpu = new AvrCore( mcu );
        else if( m_core == "Pic14" )    mcu->cpu = new Pic14Core( mcu );
        else if( m_core == "Pic14e" )   mcu->cpu = new Pic14eCore( mcu );
        else if( m_core == "8051" )     mcu->cpu = new I51Core( mcu );
        else if( m_core == "6502" )     mcu->cpu = new Mcs65Cpu( mcu );
        else if( m_core == "Z80" )      mcu->cpu = new Z80Core( mcu );
        else if( m_core == "scripted" )
        {
            ScriptCpu* cpu = new ScriptCpu( mcu );
            mcu->cpu = cpu;
            m_mcuComp->m_scripted = true;

            if( root.hasAttribute("linkable")
             && root.attribute("linkable") == "true" )
                m_mcuComp->setScriptLinkable( cpu );

            node = root.firstChild();
            while( !node.isNull() )
            {
                QDomElement e = node.toElement();
                QString  part = e.tagName();

                if( part == "properties" )
                {
                    QString group = e.attribute("name");

                    QList<ComProperty*> propList;
                    m_mcuComp->addPropGroup( {group, propList} );

                    QDomNode node = e.firstChild();
                    while( !node.isNull() )
                    {
                        QDomElement el = node.toElement();

                        if( el.tagName() == "property" )
                        {
                            QString name = el.attribute("name");
                            QString type = el.attribute("type");

                            cpu->addProperty( group, name, type );
                        }
                        node = node.nextSibling();
                    }
                }
                node = node.nextSibling();
            }
            cpu->setPeriferals( m_scriptPerif );

            cpu->setScriptFile( m_basePath+"/"+root.attribute("script") );
        }
        else mcu->cpu = new McuCpu( mcu );

        if( m_newStack ) createStack( &m_stackEl );
    }

    if( root.hasAttribute("clkpin") )
    {
        mcu->m_clkPin = mcu->getIoPin( root.attribute("clkpin") );
    }
    if( main ) m_mcuComp->setup( root.tagName() );
    return 0;
}

void McuCreator::createCfgWord( QDomElement* e )
{
    QString name = e->attribute("name");
    QString type = e->attribute("type");

    ConfigWord* cfgWord = NULL;
    if( m_core.startsWith("Pic14") ) cfgWord = PicConfigWord::createCfgWord( mcu, name, type );
    if( !cfgWord ) return;

    mcu->m_cfgWord = cfgWord;

    QDomNode node = e->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();
        if( el.tagName() == "word" )
        {
            //QString name = el.attribute("name");
            uint16_t addr  = el.attribute("address").toUInt( NULL, 16 );
            uint16_t value = el.attribute("value").toUInt( NULL, 16 );
            cfgWord->m_cfgWords.insert( addr, value );
        }
        node = node.nextSibling();
    }
}

void McuCreator::createProgMem( uint32_t size )
{
    mcu->m_flashSize = size;
    if     ( m_core == "Pic14" )  mcu->m_progMem.resize( size, 0x3FFF );
    else if( m_core == "Pic14e" ) mcu->m_progMem.resize( size, 0x3FFF );
    else                          mcu->m_progMem.resize( size, 0xFFFF );
}

void McuCreator::createDataMem( uint32_t size )
{
    mcu->m_ramSize = size;
    mcu->m_dataMem.resize( size, 0 );
    mcu->m_addrMap.resize( size, 0xFFFF ); // Not Maped values = 0xFFFF -> don't exist
}

void McuCreator::createRomMem( uint32_t size )
{
    if( size == 0 ) return;

    mcu->m_romSize = size;
    mcu->m_eeprom.resize( size );
    mcu->m_eeprom.fill( 0xFF );
}

void McuCreator::createEeprom( QDomElement* e )
{
    McuEeprom* eeprom = NULL;
    QString eepromName = e->attribute("name");

    if     ( m_core == "AVR" )    eeprom = new AvrEeprom( mcu, eepromName );
    else if( m_core == "Pic14" )  eeprom = new PicEeprom( mcu, eepromName );
    else if( m_core == "Pic14e" ) eeprom = new PicEeprom( mcu, eepromName );
    else return;

    mcu->m_modules.emplace_back( eeprom );

    setConfigRegs( e, eeprom );

    if( e->hasAttribute("dataregs") )
        eeprom->m_dataReg = mcu->getReg( e->attribute("dataregs") );

    if( e->hasAttribute("addressreg") )
    {
        QStringList regs = e->attribute("addressreg").split(",");
        QString lowByte  = regs.value(0);
        QString highByte = regs.value(1);

        if( !lowByte.isEmpty() ){
            eeprom->m_addressL = mcu->getReg( lowByte );
            watchRegNames( lowByte, R_WRITE, eeprom, &McuEeprom::addrWriteL, mcu );
        }
        if( !highByte.isEmpty() ){
            eeprom->m_addressH = mcu->getReg( highByte );
            watchRegNames( highByte, R_WRITE, eeprom, &McuEeprom::addrWriteH, mcu );
    }   }
    if( e->hasAttribute("interrupt") ) setInterrupt( e->attribute("interrupt"), eeprom );
}

void McuCreator::createDataBlock( QDomElement* d )
{
    uint16_t datStart = d->attribute("start").toUInt(0,0);
    uint16_t datEnd   = d->attribute("end").toUInt(0,0);
    uint16_t mapTo    = datStart;

    if( datEnd >= mcu->m_ramSize )
    {
        qDebug() << "McuCreator::createDataBlock  ERROR creating DataBlock";
        qDebug() << "dataMemSize  = " << mcu->m_ramSize;
        qDebug() << "dataBlockEnd = " << datEnd;
        return;
    }
    if( d->hasAttribute("mapto") ) mapTo = d->attribute("mapto").toUInt(0,0);

    for( int i=datStart; i<=datEnd; ++i )
    {
        mcu->m_addrMap[i] = mapTo;
        mapTo++;
    }
    getRegisters( d );
}

void McuCreator::createRegisters( QDomElement* e )
{
    uint16_t regStart = e->attribute("start").toUInt(0,0);
    uint16_t regEnd   = e->attribute("end").toUInt(0,0);
    uint16_t offset   = e->attribute("offset").toUInt(0,0);

    if( regEnd >= mcu->m_ramSize )
    {
        qDebug() << "McuCreator::createRegisters  ERROR creating Registers";
        qDebug() << "dataMemSize  = " << mcu->m_ramSize;
        qDebug() << "RegistersEnd = " << regEnd;
        return;
    }
    if( regStart < mcu->m_regStart ) mcu->m_regStart = regStart;
    if( regEnd   > mcu->m_regEnd )
    {
        mcu->m_regEnd = regEnd;
        mcu->m_regMask.resize( regEnd, 0xFF );
    }
    getRegisters( e, offset );
}
void McuCreator::getRegisters( QDomElement* e, uint16_t offset )
{
    QString stReg;
    if( e->hasAttribute( "streg" ) ) stReg = e->attribute( "streg" );

    QDomNode node = e->firstChild();
    while( !node.isNull() ) // Create Registers
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "register" )
        {
            QString  regName = el.attribute("name");
            uint16_t regAddr = el.attribute("addr").toUInt(0,0)+offset;

            if( regAddr >= mcu->m_ramSize )
            {
                qDebug() << "McuCreator::getRegisters  ERROR creating Register"<< regName ;
                qDebug() << "dataMemSize  = " << mcu->m_ramSize;
                qDebug() << "Register Address = " << regAddr<<"\n";
            }
            else{
                uint8_t resetVal = el.attribute("reset").toUInt(0,2);
                QString    wMask = el.attribute("mask");
                QString bits = el.attribute( "bits" );

                if( !wMask.isEmpty() ) mcu->m_regMask[regAddr] = wMask.toUInt(0,2);
                mcu->m_addrMap[regAddr] = regAddr;

                regInfo_t regInfo = { regAddr, resetVal/*, writeMask*/ };
                mcu->m_regInfo.insert( regName, regInfo );

                if( !bits.isEmpty() )                    // Create bitMasks
                {
                    QString bitName;
                    QStringList bitList = bits.split(",");
                    for( int i=0; i<bitList.size(); ++i )
                    {
                        bitName = bitList.value( i );
                        if( bitName == "0" ) continue;
                        for( QString alias : bitName.split("|") ) // Bit name variations: alias first used bit name
                        {                                         // Example tiny WDTCR.WDTIE is 328 WDTCSR.WDIE
                            mcu->m_bitMasks.insert( alias, 1<<i );
                            mcu->m_bitRegs.insert( alias, regAddr );
                    }   }
                    if( !stReg.isEmpty() && ( regName == stReg ) )
                    {
                        mcu->m_sregAddr = regAddr;
                        mcu->setStatusBits( bitList );
                    }
                }
            }
        }
        else if( el.tagName() == "mapped" )
        {
            uint16_t regAddr = el.attribute("addr").toUInt(0,0)+offset;
            uint16_t mapTo   = el.attribute("mapto").toUInt(0,0);
            mcu->m_addrMap[regAddr] = mapTo;
        }
        node = node.nextSibling();
}   }

void McuCreator::createProgBlock( QDomElement* p )
{
    QDomNode node = p->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();
        if( el.tagName() == "progval" )
        {
            uint16_t addr  = el.attribute("addr").toUInt(0,0);
            uint16_t value = el.attribute("value").toUInt(0,0);

            if( addr >= mcu->m_flashSize )
            {
                qDebug() << "McuCreator::createProgBlock  ERROR writing Program Memory";
                qDebug() << "Address:" << addr << "> PMG End:" << mcu->m_flashSize;
            }
            else mcu->setFlashValue( addr, value );
        }
        node = node.nextSibling();
}   }

void McuCreator::createIntOsc( QDomElement* p )
{
    QString name = p->attribute("name");
    QString type = p->attribute("type");

    McuIntOsc* intOsc = NULL;
    if( m_core.startsWith("Pic14") ) intOsc = PicIntOsc::createIntOsc( mcu, name, type );
    else                             intOsc = new McuIntOsc( mcu, "intOsc");

    mcu->m_modules.emplace_back( intOsc );
    mcu->m_intOsc = intOsc;

    setConfigRegs( p, intOsc );

    if( p->hasAttribute("clockpins") )
    {
        if( !mcu->m_intOsc )
        {
            McuIntOsc* intOsc = new McuIntOsc( mcu, "intOsc");
            mcu->m_modules.emplace_back( intOsc );
            mcu->m_intOsc = intOsc;
        }
        QStringList pins = p->attribute("clockpins").split(",");
        for( int i=0; i<pins.size(); ++i )
            mcu->m_intOsc->setPin( i, mcu->getMcuPin( pins.value(i) ) );
    }
    if( p->hasAttribute("clockoutpin") )
    {
        mcu->m_intOsc->setPin( 2, mcu->getMcuPin( p->attribute("clockoutpin") ) );
    }
}

void McuCreator::createIoPort( QDomElement* p )
{
    QString name = p->attribute("name");

    IoPort* port = new IoPort( name );
    mcu->m_ioPorts.insert( name, port );

    port->createPins( m_mcuComp, p->attribute("pins"), 0xFFFFFFFF );
}

void McuCreator::createMcuPort( QDomElement* p )
{
    QString name = p->attribute("name");

    McuPort* port;
    if     ( m_core == "AVR" )    port = new AvrPort( mcu, name );
    else if( m_core == "Pic14" )  port = new PicPort( mcu, name );
    else if( m_core == "Pic14e" ) port = new PicPort( mcu, name );
    else if( m_core == "8051" )   port = new I51Port( mcu, name );
    else{
        if( p->hasAttribute("type") && ( p->attribute("type") == "scripted" ))
        {
            port = new ScriptPort( mcu, name );
            ScriptPort* sPort = (ScriptPort*)port;
            sPort->setScriptFile( m_basePath+"/"+p->attribute("script"));
        }
        else port = new McuPort( mcu, name );
    }
    mcu->m_mcuPorts.insert( name, port );
    mcu->m_modules.emplace_back( port );
    /// if( name.startsWith("C") ) mcu->m_ctrlPort = port;

    uint32_t pinMask=0xFFFFFFFF;
    if( p->hasAttribute("pinmask") ) pinMask = p->attribute("pinmask").toUInt( 0, 2 );
    port->createPins( m_mcuComp, p->attribute("pins"), pinMask );

    setConfigRegs( p, port );

    if( p->hasAttribute("resetpin") )
        m_mcuComp->m_portRstPin = port->getPin( p->attribute("resetpin") );

    QString Oreg = "";
    if( p->hasAttribute( "outreg" ) )
    {
        QStringList outRegList = p->attribute( "outreg" ).split(",");
        Oreg = outRegList.first();
        for( QString outReg : outRegList )
        {
            uint16_t addr = mcu->getRegAddress( outReg );
            port->m_outAddr = addr;
            port->m_outReg  = mcu->getReg( outReg );
            watchRegNames( outReg, R_WRITE, port, &McuPort::outChanged, mcu );
    }   }
    if( p->hasAttribute( "inreg" ) ) // Connect to PORT In Register
    {
        QString inReg = p->attribute( "inreg" );
        uint16_t inAddr = mcu->getRegAddress( inReg );
        port->m_inAddr = inAddr;
        port->m_inReg  = mcu->getReg( inReg );
        if( m_core == "AVR" ) watchRegister( inAddr, R_WRITE, (AvrPort*)port, &AvrPort::pinRegChanged, mcu );
    }
    else if( !Oreg.isEmpty() ) watchRegNames( Oreg, R_READ, port, &McuPort::readPort, mcu ); // No Input register, read pin states

    if( p->hasAttribute( "dirreg" ) ) // Connect to PORT Dir Register
    {
        QString dirreg = p->attribute( "dirreg" );
        if( dirreg.startsWith("!") )
        {
            port->m_dirInv = true;
            dirreg.remove( 0, 1 );
        }
        uint16_t addr = mcu->getRegAddress( dirreg );
        port->m_dirAddr = addr;
        port->m_dirReg  = mcu->getReg( dirreg );
        watchRegNames( dirreg, R_WRITE, port, &McuPort::dirChanged, mcu );
    }
    if( p->hasAttribute("outmask") ) // Permanent Outputs
    {
        uint outMask = p->attribute("outmask").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            port->m_pins[i]->m_outMask = outMask & 1<<i;
    }
    if( p->hasAttribute("inpmask") ) // Permanent Inputs
    {
        uint inpMask = p->attribute("inpmask").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            port->m_pins[i]->m_inpMask = inpMask & 1<<i;
    }
    if( p->hasAttribute("pullups") ) // Permanent Pullups
    {
        QString pullups = p->attribute( "pullups" );
        bool ok = false;
        uint pullup = pullups.toUInt( &ok, 2 );
        if( ok ){
            for( int i=0; i<port->m_numPins; ++i )
                port->m_pins[i]->m_puMask = pullup & 1<<i;
        }else{
            if( mcu->regExist( pullups ) )
                watchRegNames( pullups, R_WRITE, port, &McuPort::setPullups, mcu );
            else{
                if( pullups.startsWith("!") ){
                    pullups = pullups.remove( 0,1 );
                    watchBitNames( pullups, R_WRITE, port, &McuPort::clearAllPullups, mcu );
                }
                else watchBitNames( pullups, R_WRITE, port, &McuPort::setAllPullups, mcu );
            }
        }
    }
    if( p->hasAttribute("opencol") ) // OPen Drain
    {
        uint opencol = p->attribute("opencol").toUInt( 0, 2 );
        for( int i=0; i<port->m_numPins; ++i )
            if( opencol & 1<<i ) port->m_pins[i]->m_openColl = true;
    }
    QDomNode node = p->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "interrupt" )
        {
            setInterrupt( el.attribute("name"), port );  // Pin change interrupt

            if( el.hasAttribute("mask") )
            {
                QString mask = el.attribute( "mask" );
                bool ok = false;
                int bits = mask.toInt( &ok,2 );
                port->rstIntMask( !ok );
                if( ok ) port->setIntMask( bits ); // Hardcode Interrupt to Pin
                else                               // Interrupt controlled by register
                {
                    watchRegNames( mask, R_WRITE, port, &McuPort::intChanged, mcu );
                }
            }
            else if(  el.hasAttribute("bitmask") )
            {
                QString bitNames = el.attribute( "bitmask" );
                port->m_intBits = getRegBits( bitNames, mcu );
                watchBitNames( bitNames, R_WRITE, port, &McuPort::intChanged, mcu );
            }
        }
        else if( el.tagName() == "extint" )
        {
            QString intName = el.attribute("name");
            Interrupt* inte = mcu->m_interrupts.m_intList.value( intName );

            McuPin* pin = port->getPin( el.attribute("pin") );
            if( pin ) pin->m_extInt = inte;

            if( el.hasAttribute("configbits") )
            {
                QString configBits = el.attribute("configbits");
                watchBitNames( configBits, R_WRITE, pin, &McuPin::ConfExtInt, mcu );
                if( pin ) pin->m_extIntBits = getRegBits( configBits, mcu );
        }   }
        node = node.nextSibling();
}   }

void McuCreator::createTimer( QDomElement* t )
{
    McuTimer* timer = NULL;
    QString timerName = t->attribute("name");
    int type = t->attribute("type").toInt();

    if     ( m_core == "8051" )   timer = new I51Timer( mcu, timerName );
    else if( m_core == "AVR" )    timer = AvrTimer::createTimer( mcu, timerName, type );
    else if( m_core == "Pic14" )  timer = PicTimer::createTimer( mcu, timerName, type );
    else if( m_core == "Pic14e" ) timer = PicTimer::createTimer( mcu, timerName, type );

    if( !timer ) { qDebug() << "Error creating Timer"<< timerName; return; }

    mcu->m_timerList.insert( timerName, timer );
    mcu->m_modules.emplace_back( timer );

    setConfigRegs( t, timer );

    if( t->hasAttribute("counter") )
    {
        QStringList regs = t->attribute("counter").split(",");
        QString lowByte  = regs.value(0);
        QString highByte = regs.value(1);

        if( !lowByte.isEmpty() ){
            timer->m_countL = mcu->getReg( lowByte );
            watchRegNames( lowByte, R_WRITE, timer, &McuTimer::countWriteL, mcu );
            watchRegNames( lowByte, R_READ,  timer, &McuTimer::updtCount, mcu );
        }
        if( !highByte.isEmpty() ){
            timer->m_countH = mcu->getReg( highByte );
            watchRegNames( highByte, R_WRITE, timer, &McuTimer::countWriteH, mcu );  // Low byte triggers read/write operations
            watchRegNames( highByte, R_READ,  timer, &McuTimer::updtCount, mcu );
    }   }
    if( t->hasAttribute("enable") )
    {
        QString enable = t->attribute("enable");
        watchBitNames( enable, R_WRITE, timer, &McuTimer::enable, mcu );
    }
    if( t->hasAttribute("clockpin") )
        timer->m_clockPin = mcu->getMcuPin( t->attribute("clockpin") );

    QString topReg0 = "";
    if( t->hasAttribute("topreg0") ) /// Still done in AvrTimer16bit
    {
        topReg0 = t->attribute("topreg0");
        QStringList list = topReg0.split(",");

        timer->m_topReg0L = mcu->getReg( list.value(0) );
        watchRegNames( topReg0, R_WRITE, timer, &McuTimer::topReg0Changed, mcu );

        if( list.size() > 1 ) timer->m_topReg0H = mcu->getReg( list.value(1) );
    }
    if( t->hasAttribute("interrupt") ) setInterrupt( t->attribute("interrupt"), timer );
    if( t->hasAttribute("prescalers") ) setPrescalers( t->attribute("prescalers"), timer );
    if( t->hasAttribute("prselect") ) timer->m_prSelBits = getRegBits( t->attribute("prselect"), mcu );

    QDomNode node = t->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "ocunit" )
        {
            QString ocName = el.attribute("name");
            McuOcUnit* ocUnit = NULL;
            if     ( m_core == "AVR" ) ocUnit = new AvrOcUnit( mcu, ocName );
            //else if( m_core == "Pic14" ) ocUnit = new PicOcUnit( mcu, el.attribute("name") );
            if( !ocUnit ) { node = node.nextSibling(); continue; }

            timer->addOcUnit( ocUnit );
            ocUnit->m_timer = timer;
            ocUnit->m_ocPin = mcu->getMcuPin( el.attribute("pin") );

            QStringList regs = el.attribute("ocreg").split(",");
            QString lowByte = regs.value(0);
            QString highByte =regs.value(1);

            if( !lowByte.isEmpty() ){
                uint8_t* ocRegL = mcu->getReg( lowByte );
                ocUnit->m_ocRegL = ocRegL;
                if( topReg0 == "" && ocName.endsWith("A") ) // OCRA managed in Timer
                {
                    timer->m_topReg0L = ocRegL;
                    watchRegNames( lowByte, R_WRITE, timer, &McuTimer::topReg0Changed, mcu );
                }
                else watchRegNames( lowByte, R_WRITE, ocUnit, &McuOcUnit::ocrWriteL, mcu );
            }
            if( !highByte.isEmpty() ){
                uint8_t* ocRegH = mcu->getReg( highByte );
                ocUnit->m_ocRegH = ocRegH;
                if( ocName.endsWith("A") ) timer->m_topReg0H = ocRegH;
                /// Low byte triggers red/write operations
                watchRegNames( highByte, R_WRITE, ocUnit, &McuOcUnit::ocrWriteH, mcu );
            }
            if( el.hasAttribute("bits") ) // This doesn't watch register, configure must be called from Timer
            {
                ocUnit->m_configBitsA = getRegBits( el.attribute("bits"), mcu );
            }
            else setConfigRegs( &el, ocUnit );

            if( el.hasAttribute("interrupt") ) setInterrupt( el.attribute("interrupt"), ocUnit );
        }
        else if( el.tagName() == "icunit" )
        {
            QString icName = el.attribute("name");
            McuIcUnit* icUnit = NULL;
            if( m_core == "AVR" ) icUnit = new AvrIcUnit( mcu, icName );
            if( !icUnit ) { node = node.nextSibling(); continue; }

            timer->m_ICunit = icUnit;
            icUnit->m_timer = timer;
            icUnit->m_icPin = mcu->getMcuPin( el.attribute("pin") );

            QStringList regs = el.attribute("icreg").split(",");
            if( !regs.isEmpty() ) icUnit-> m_icRegL = mcu->getReg( regs.value(0) );
            if( !regs.isEmpty() ) icUnit-> m_icRegH = mcu->getReg( regs.value(1) );

            if( el.hasAttribute("bits") )
            {
                QString configBits = el.attribute("configbits");
                watchBitNames( configBits, R_WRITE, icUnit, &McuIcUnit::configure, mcu );
            }
            if( el.hasAttribute("interrupt") ) setInterrupt( el.attribute("interrupt"), icUnit );
        }
        node = node.nextSibling();
}   }

void McuCreator::createOcm( QDomElement* e )
{
    McuOcm* ocm = NULL;
    QString name = e->attribute("name");

    if( m_core == "AVR" ) ocm = new AvrOcm( mcu, name );
    if( !ocm ) return;

    mcu->m_modules.emplace_back( ocm );
    setConfigRegs( e, ocm );

    QStringList ocunits = e->attribute("ocunits").split(",");
    for( int i=0; i<2; i++ )
    {
        QString ocunit = ocunits.takeFirst();
        McuTimer* timer = mcu->getTimer("TIMER"+ocunit.mid( 2, 1) );

        McuOcUnit* ocUnit = timer->getOcUnit( ocunit );
        ocUnit->m_ocm = ocm;
        if( i == 0 ) ocm->m_OC1 = ocUnit;
        else         ocm->m_OC2 = ocUnit;
    }
}

void McuCreator::createCcpUnit( QDomElement* c )
{
    PicCcpUnit* ccpUnit = NULL;
    if     ( m_core == "Pic14" ) ccpUnit = new PicCcpUnit( mcu, c->attribute("name"), c->attribute("type").toInt() );
    else if( m_core == "Pic14e") ccpUnit = new PicCcpUnit( mcu, c->attribute("name"), c->attribute("type").toInt() );
    if( !ccpUnit ) return;

    mcu->m_modules.emplace_back( ccpUnit );

    ccpUnit->setPin( mcu->getMcuPin( c->attribute("pin") ) );

    QStringList regs = c->attribute("ccpreg").split(",");
    QString lowByte  = regs.value(0);
    QString highByte = regs.value(1);

    if( !lowByte.isEmpty() ){
        uint8_t* regL = mcu->getReg( lowByte );
        ccpUnit->m_ccpRegL = regL;
        ccpUnit->m_capUnit->m_icRegL = regL;
        watchRegNames( lowByte, R_WRITE, ccpUnit, &PicCcpUnit::ccprWriteL, mcu );
    }
    if( !highByte.isEmpty() ){
        uint8_t* regH = mcu->getReg( highByte );
        ccpUnit->m_ccpRegH = regH;
        ccpUnit->m_capUnit->m_icRegH = regH;
        watchRegNames( highByte, R_WRITE, ccpUnit, &PicCcpUnit::ccprWriteH, mcu );
    }
    setConfigRegs( c, ccpUnit );

    if( c->hasAttribute("interrupt") ) setInterrupt( c->attribute("interrupt"), ccpUnit );
}

void McuCreator::createMsspUnit( QDomElement* c )
{
    PicMssp* msspUnit = new PicMssp(  mcu, c->attribute("name"), c->attribute("type").toInt() );

    mcu->m_modules.emplace_back( msspUnit );
    setConfigRegs( c, msspUnit );

    QDomNode node = c->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();
        if     ( el.tagName() == "twi") createTwi( &el );
        else if( el.tagName() == "spi") createSpi( &el );
        node = node.nextSibling();
    }
    msspUnit->m_twiUnit = static_cast<PicTwi*>( m_twi );
    msspUnit->m_spiUnit = static_cast<PicSpi*>( m_spi );
}

void McuCreator::createUsart( QDomElement* u )
{
    QString name = u->attribute( "name" );
    int   number = u->attribute( "number" ).toInt();

    QString core = m_core;
    if( u->hasAttribute("core") ) core = u->attribute("core");

    McuUsart* usartM;
    if     ( core == "8051" )    usartM = new I51Usart( mcu, name, number );
    else if( core == "AVR"  )    usartM = new AvrUsart( mcu, name, number );
    else if( core == "Pic14")    usartM = new PicUsart( mcu, name, number );
    else if( core == "Pic14e")   usartM = new PicUsart( mcu, name, number );
    else if( core == "scripted"){
        ScriptUsart* su = new ScriptUsart( mcu, name, number );
        usartM = su;
        m_scriptPerif.push_back( su );
    }
    else return;

    mcu->m_usarts.emplace_back( usartM );
    mcu->m_modules.emplace_back( usartM );

    setConfigRegs( u, usartM );
    if( u->hasAttribute("interrupt") ) setInterrupt( u->attribute("interrupt"), usartM );

    QDomNode node = u->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();

        if( el.tagName() == "trunit" )
        {
            UartTR* trUnit = NULL;
            QString type = el.attribute( "type" );
            if( type == "tx" )
            {
                trUnit = usartM->m_sender;
                m_txRegName = el.attribute( "register" );
                watchRegNames( m_txRegName, R_WRITE, usartM, &McuUsart::sendByte, mcu );
            }
            else if( type == "rx" )
            {
                trUnit = usartM->m_receiver;
                QString regName = el.attribute( "register" );
                if( regName.isEmpty() ) regName = m_txRegName; // Tx and rx using the same register
                watchRegNames( regName, R_READ, usartM, &McuUsart::readByte, mcu );
            }
            if( trUnit )
            {
                setConfigRegs( &el, trUnit );

                QStringList pinNames = el.attribute( "pin" ).split(",");
                QList<IoPin*> pinList;
                for( QString pinName : pinNames ) pinList.append( mcu->getIoPin( pinName ) );
                trUnit->setPins( pinList );

                if( el.hasAttribute("enable") )
                {
                    QString enable = el.attribute( "enable" );
                    watchBitNames( enable, R_WRITE, trUnit, &UartTR::enable, mcu );
                }
                if( el.hasAttribute("interrupt") ) setInterrupt( el.attribute("interrupt"), trUnit );
            }
        }
        node = node.nextSibling();
}   }

void McuCreator::createAdc( QDomElement* e )
{
    McuAdc* adc = NULL;
    QString name = e->attribute( "name" );
    int type = e->attribute("type").toInt();

    if     ( m_core == "AVR" )   adc = AvrAdc::createAdc( mcu, name, type );
    else if( m_core == "Pic14")  adc = PicAdc::createAdc( mcu, name, type );
    else if( m_core == "Pic14e") adc = PicAdc::createAdc( mcu, name, type );
    if( !adc ) return;

    mcu->m_modules.emplace_back( adc );

    setConfigRegs( e, adc );

    if( e->hasAttribute("bits") )
    {
        bool ok = false;
        int bits = e->attribute("bits").toInt( &ok );
        if( ok ) adc->m_maxValue = pow( 2, bits )-1;
    }
    if( e->hasAttribute("dataregs") )
    {
        QStringList regs = e->attribute("dataregs").split(",");
        QString lowByte  = regs.value(0);
        QString highByte = regs.value(1);

        if( !lowByte.isEmpty() )  adc->m_ADCL = mcu->getReg( lowByte );
        if( !highByte.isEmpty() ) adc->m_ADCH = mcu->getReg( highByte );
    }
    if( e->hasAttribute("multiplex") )
    {
        QString configRegs = e->attribute("multiplex");
        watchRegNames( configRegs, R_WRITE, adc, &McuAdc::setChannel, mcu );
    }
    if( e->hasAttribute("interrupt") ) setInterrupt( e->attribute("interrupt"), adc );
    if( e->hasAttribute("prescalers") ) setPrescalers( e->attribute("prescalers"), adc );

    QStringList pins = e->attribute("adcpins").remove(" ").split(",");
    for( QString pinName : pins )
    {
        McuPin* pin = mcu->getMcuPin( pinName );
        if( pin ) adc->m_adcPin.emplace_back( pin );
    }
    if( e->hasAttribute("vrefpins") )
    {
        pins = e->attribute("vrefpins").remove(" ").split(",");
        for( QString pinName : pins )
        {
            McuPin* pin = mcu->getMcuPin( pinName );
            if( pin ) adc->m_refPin.emplace_back( pin );
    }   }
}

void McuCreator::createAcomp( QDomElement* e )
{
    McuComp* comp = NULL;
    QString name = e->attribute( "name" );
    int type = e->attribute("type").toInt();

    if     ( m_core == "AVR" )    comp = new AvrComp( mcu, name );
    else if( m_core == "Pic14" )  comp = PicComp::createComparator( mcu, name, type );
    else if( m_core == "Pic14e" ) comp = PicComp::createComparator( mcu, name, type );
    if( !comp ) return;

    mcu->m_modules.emplace_back( comp );
    mcu->m_comparator = comp;
    setConfigRegs( e, comp );

    QStringList pins = e->attribute( "pins" ).split(",");
    for( QString pinName : pins )
    {
        McuPin* pin = mcu->getMcuPin( pinName );
        if( pin ) comp->m_pins.emplace_back( pin );
    }
    if( e->hasAttribute("interrupt") ) setInterrupt( e->attribute("interrupt"), comp );
}

void McuCreator::createVref( QDomElement* e )
{
    QString name = e->attribute( "name" );
    McuVref* vref = NULL;
    if     ( m_core == "Pic14" )  vref = new PicVref( mcu, name );
    else if( m_core == "Pic14e" ) vref = new PicVref( mcu, name );
    if( !vref ) return;

    mcu->m_modules.emplace_back( vref );
    mcu->m_vrefModule = vref;

    setConfigRegs( e, vref );

    if( e->hasAttribute("pinout") )
        vref->m_pinOut = mcu->getMcuPin( e->attribute("pinout") );
}

void McuCreator::createTwi( QDomElement* e )
{
    QString name = e->attribute( "name" );

    if     ( m_core == "AVR" )  m_twi = new AvrTwi( mcu, name );
    else if( m_core == "Pic14") m_twi = new PicTwi( mcu, name );
    else return;

    mcu->m_modules.emplace_back( m_twi );

    setConfigRegs( e, m_twi );

    QString dataReg = e->attribute("dataregs");
    m_twi->m_dataReg = mcu->getReg( dataReg );
    watchRegNames( dataReg, R_WRITE, m_twi, &McuTwi::writeTwiReg, mcu );
    if( m_core == "Pic14")
        watchRegNames( dataReg, R_READ, m_twi, &McuTwi::readTwiReg, mcu ); //

    QString addrReg = e->attribute("addressreg");
    m_twi->m_addrReg = mcu->getReg( addrReg );
    watchRegNames( addrReg, R_WRITE, m_twi, &McuTwi::writeAddrReg, mcu );

    QString statReg = e->attribute("statusreg");
    m_twi->m_statReg = mcu->getReg( statReg );
    watchRegNames( statReg, R_WRITE, m_twi, &McuTwi::writeStatus, mcu );

    if( e->hasAttribute("interrupt") ) setInterrupt( e->attribute("interrupt"), m_twi );
    if( e->hasAttribute("prescalers") ) setPrescalers( e->attribute("prescalers"), m_twi );

    QStringList pins = e->attribute("pins").remove(" ").split(",");
    m_twi->setSdaPin( mcu->getMcuPin( pins.value(0) ) );
    m_twi->setSclPin( mcu->getMcuPin( pins.value(1) ) );
}

void McuCreator::createSpi( QDomElement* e )
{
    QString name = e->attribute( "name" );

    if     ( m_core == "AVR" )  m_spi = new AvrSpi( mcu, name );
    else if( m_core == "Pic14") m_spi = new PicSpi( mcu, name );
    else if( m_core == "scripted"){
        ScriptSpi* su = new ScriptSpi( mcu, name );
        m_spi = su;
        m_scriptPerif.push_back( su );
    }
    else return;

    mcu->m_modules.emplace_back( m_spi );

    setConfigRegs( e, m_spi );

    if(  e->hasAttribute("dataregs") )
    {
        QString dataReg = e->attribute("dataregs");
        m_spi->m_dataReg = mcu->getReg( dataReg );
        watchRegNames( dataReg, R_WRITE, m_spi, &McuSpi::writeSpiReg, mcu );
    }
    if(  e->hasAttribute("statusreg") )
    {
        QString statReg = e->attribute("statusreg");
        m_spi->m_statReg = mcu->getReg( statReg );
        watchRegNames( statReg, R_WRITE, m_spi, &McuSpi::writeStatus, mcu );
    }
    if( e->hasAttribute("interrupt")  ) setInterrupt(  e->attribute("interrupt") , m_spi );
    if( e->hasAttribute("prescalers") ) setPrescalers( e->attribute("prescalers"), m_spi );

    QStringList pins = e->attribute("pins").remove(" ").split(",");
    m_spi->setMosiPin( mcu->getMcuPin( pins.value(0) ) );
    m_spi->setMisoPin( mcu->getMcuPin( pins.value(1) ) );
    m_spi->setSckPin(  mcu->getMcuPin( pins.value(2) ) );
    m_spi->setSsPin(   mcu->getMcuPin( pins.value(3) ) );
}

void McuCreator::createWdt( QDomElement* e )
{
    QString name = e->attribute( "name" );
    McuWdt* wdt;
    if     ( m_core == "AVR" )   wdt = AvrWdt::createWdt( mcu, name );
    else if( m_core == "Pic14" ) wdt = new PicWdt( mcu, name );
    else if( m_core == "Pic14e") wdt = new PicWdt( mcu, name );
    else return;

    mcu->m_modules.emplace_back( wdt );
    mcu->m_wdt = wdt;

    setConfigRegs( e, wdt );

    if( e->hasAttribute("interrupt") ) setInterrupt( e->attribute("interrupt"), wdt );
    if( e->hasAttribute("prescalers") ) setPrescalers( e->attribute("prescalers"), wdt );
}

void McuCreator::createSleep( QDomElement* e )
{
    QString name = e->attribute( "name" );
    McuSleep* sleep;
    if     ( m_core == "AVR" ) sleep = new AvrSleep( mcu, name );
    else return;

    mcu->m_modules.emplace_back( sleep );
    mcu->m_sleepModule = sleep;

    setConfigRegs( e, sleep );
}

/*void McuCreator::createExtMem( QDomElement* e )
{
    QString name = e->attribute( "name" );
    ExtMemModule* extMem = new ExtMemModule( mcu, name );

    mcu->extMem = extMem;
    mcu->m_modules.emplace_back( extMem );

    McuPort* addrPort = mcu->getPort( e->attribute("addrport") );
    for( int i=0; i<addrPort->m_numPins; ++i )
    {
        McuPin* pin = addrPort->getPinN( i );
        if( pin )
        {
            extMem->m_addrPin.emplace_back( pin );
            pin->setDirection( true );
            pin->controlPin( true, true );
        }
    }
    McuPort* dataPort = mcu->getPort( e->attribute("dataport") );
    for( int i=0; i<dataPort->m_numPins; ++i )
    {
        McuPin* pin = dataPort->getPinN( i );
        if( pin )
        {
            extMem->m_dataPin.emplace_back( pin );
            pin->setDirection( false );
            pin->controlPin( true, true );
        }
    }
    extMem->m_rwPin = mcu->getPin( e->attribute("rwpin") );
    extMem->m_rePin = mcu->getPin( e->attribute("repin") );
    extMem->m_enPin = mcu->getPin( e->attribute("enpin") );
    extMem->m_laPin = mcu->getPin( e->attribute("lapin") );
}*/

void McuCreator::createIntMem( QDomElement* e )
{
    QString name = e->attribute( "name" );
    IntMemModule* intMem = new IntMemModule( mcu, name );

    mcu->m_modules.emplace_back( intMem );

    IoPort* addrPort = mcu->getIoPort( e->attribute("addrport") );
    for( int i=0; i<addrPort->m_numPins; ++i )
    {
        IoPin* pin = addrPort->getPinN( i );
        if( pin ){
            intMem->m_addrPin.emplace_back( pin );
            pin->setPinMode( input );
            //pin->controlPin( true, true );
        }
    }
    IoPort* dataPort = mcu->getIoPort( e->attribute("dataport") );
    for( int i=0; i<dataPort->m_numPins; ++i )
    {
        IoPin* pin = dataPort->getPinN( i );
        if( pin ){
            intMem->m_dataPin.emplace_back( pin );
            pin->setPinMode( input );
            //pin->controlPin( true, true );
        }
    }
    intMem->m_rwPin  = mcu->getIoPin( e->attribute("rwpin") );
    intMem->m_cshPin = mcu->getIoPin( e->attribute("cshpin") );
    intMem->m_cslPin = mcu->getIoPin( e->attribute("cslpin") );
    intMem->m_clkPin = mcu->getIoPin( e->attribute("clkpin") );
}

void McuCreator::createStack( QDomElement* s )
{
    QStringList spRegs = s->attribute("spreg").split(",");
    mcu->cpu->m_spl = mcu->getReg( spRegs.value(0) );
    if( spRegs.size() > 1 )
        mcu->cpu->m_sph = mcu->getReg( spRegs.value(1) );

    QString inc = s->attribute("increment");

    mcu->cpu->m_spPre = inc.contains("pre");
    mcu->cpu->m_spInc = inc.contains("inc") ? 1:-1;
}

void McuCreator::createInterrupts( QDomElement* i )
{
    QString enable = i->attribute("enable");
    if( !enable.isEmpty() )
    {
        mcu->m_interrupts.m_enGlobalFlag = getRegBits( enable, mcu );
        watchBitNames( enable, R_WRITE, mcu, &eMcu::enableInterrupts, mcu );
    }
    QDomNode node = i->firstChild();
    while( !node.isNull() )
    {
        QDomElement el = node.toElement();
        if( el.tagName() == "interrupt" ) createInterrupt( &el );
        node = node.nextSibling();
}   }

void McuCreator::createInterrupt( QDomElement* el )
{
    QString  intName = el->attribute("name");
    if( intName.isEmpty() ) return;

    uint16_t intVector  = el->attribute("vector").toUInt(0,0);

    Interrupt* iv = NULL;
    if     ( m_core == "8051" )  iv = I51Interrupt::getInterrupt( intName, intVector, mcu );
    //else if( m_core == "AVR" )   iv = AVRInterrupt::getInterrupt( intName, intVector, mcu );
    else if( m_core == "Pic14" ) iv = new PicInterrupt( intName, intVector, mcu );
    else if( m_core == "Pic14e") iv = new Pic14eInterrupt( intName, intVector, mcu );
    else                         iv = new Interrupt( intName, intVector, mcu );
    if( !iv ) return;

    mcu->m_interrupts.m_intList.insert( intName, iv );
    iv->m_interrupts = &(mcu->m_interrupts);

    QString enable = el->attribute("enable");
    if( !enable.isEmpty() ) watchBitNames( enable, R_WRITE, iv, &Interrupt::enableFlag, mcu );

    QString intFlag = el->attribute("flag");
    iv->m_flagMask = mcu->m_bitMasks.value( intFlag );
    iv->m_flagReg  = mcu->m_bitRegs.value( intFlag );

    QString intPrio = el->attribute("priority");
    bool ok = false;
    uint8_t prio = intPrio.toUInt(&ok,0);

    if( ok ) iv->setPriority( prio );
    else     watchBitNames( intPrio, R_WRITE, iv, &Interrupt::setPriority, mcu );

    if( m_core != "AVR" ) /// TODO find a generic way
    {
        watchBitNames( intFlag, R_WRITE, iv, &Interrupt::flagCleared, mcu );
    }
    if( el->hasAttribute("clear") ) // If clear="1" Clear flag by writting 1 to it
    {                               // If clear="0" deactivate autoclear
        uint8_t val = el->attribute("clear").toUInt();
        if( val ) watchBitNames( intFlag, R_WRITE, iv, &Interrupt::writeFlag, mcu );
        else iv->m_autoClear = false;
    }
    if( el->hasAttribute("pin") )
    {
        iv->m_intPin = mcu->getIoPin( el->attribute("pin") );
    }
    if( el->hasAttribute("wakeup") )
    {
        iv->m_wakeup = el->attribute("wakeup").toUInt( 0, 2 );
}   }

void McuCreator::setInterrupt( QString intName, McuModule* module )
{
    Interrupt* inte = mcu->m_interrupts.m_intList.value( intName );
    module->setInterrupt( inte );
}

void McuCreator::setConfigRegs( QDomElement* u, McuModule* module )
{
    if( u->hasAttribute("configregsA") )
    {
        QString regs = u->attribute("configregsA");
        watchRegNames( regs, R_WRITE, module, &McuModule::configureA, mcu );
    }
    if( u->hasAttribute("configregsB") )
    {
        QString regs = u->attribute("configregsB");
        watchRegNames( regs, R_WRITE, module, &McuModule::configureB, mcu );
    }
    if( u->hasAttribute("configregsC") )
    {
        QString regs = u->attribute("configregsC");
        watchRegNames( regs, R_WRITE, module, &McuModule::configureC, mcu );
    }
    if( u->hasAttribute("configbitsA") )
    {
        QString configBits = u->attribute("configbitsA");
        watchBitNames( configBits, R_WRITE, module, &McuModule::configureA, mcu );
        module->m_configBitsA = getRegBits( configBits, mcu );
    }
    if( u->hasAttribute("configbitsB") )
    {
        QString configBits = u->attribute("configbitsB");
        watchBitNames( configBits, R_WRITE, module, &McuModule::configureB, mcu );
        module->m_configBitsB = getRegBits( configBits, mcu );
    }
    if( u->hasAttribute("configbitsC") )
    {
        QString configBits = u->attribute("configbitsC");
        watchBitNames( configBits, R_WRITE, module, &McuModule::configureC, mcu );
        module->m_configBitsC = getRegBits( configBits, mcu );
}   }

void McuCreator::setPrescalers( QString pr, McuPrescaled* module )
{
    QStringList prescalers = pr.remove(" ").split(",");
    module->m_prescList.resize( prescalers.size() );

    for( int i=0; i<prescalers.size(); ++i )
        module->m_prescList[i] = prescalers.value(i).toUInt();
}

void McuCreator::convert( QString fileName ) // TODELETE convert dat files to xml reg files.
{
    QStringList lines = fileToStringList( fileName, "McuCreator::convert" );
    bool regs = false;
    bool bits = false;
    int max = 0;
    int min = 999;
    QMap<int, QString> regMap;
    QHash<QString,QVector<QString> > bitHash;
    for( QString line : lines )
    {
        if( line.startsWith("'") ) continue;
        if( regs )
        {
            if( line.remove(" ").isEmpty() ) continue;
            if( line.contains("[Bits]") ) { bits = true; regs = false; continue;}
            QStringList words = line.split(",");
            regMap[words.value(1).toInt()] = words.value(0);
        }
        else if( bits )
        {
            if( line.remove(" ").isEmpty() ) continue;
            if( line.contains("[") ) break;
            QStringList words = line.split(",");
            QString bit = words.value(0);
            QString reg = words.value(1);

            QVector<QString> bh = bitHash[reg];
            if( bh.isEmpty() )  bh.fill("0",8);

            int i = words.value(2).toInt();
            if( i > max ) max = i;
            if( i < min ) min = i;

            bh[i] = bit;
            bitHash[reg] = bh;
        }
        if( line.contains("[Registers]") ) regs = true;
    }
    QString doc;
    doc.append("<parts>\n");

    QList<int> addresses = regMap.keys();

    for( int addr : addresses )
    {
        doc.append("    <register ");

        QString regName = regMap.value(addr);
        QString ad = QString::number(addr,16).toUpper();
        while( ad.size() < 4 ) ad.prepend("0");
        QString nameEntry = " name=\""+regName+"\" ";
        while( nameEntry.size() < 15 ) nameEntry.append(" ");

        doc.append( nameEntry );
        doc.append(" addr=\"0x"+ad+"\" ");
        doc.append(" reset=\"\" ");
        doc.append(" mask=\"\" ");

        QString bitsList;
        QVector<QString> bitVec = bitHash.value(regName);
        if( !bitVec.isEmpty() )
        {
            for( int i=0; i<8; i++ )
            {
                QString bitName = bitVec.value(i);
                if( bitName.isEmpty() ) continue;
                bitsList.append( bitName+",");
            }
            bitsList.remove(bitsList.size()-1,1);
            doc.append("\n               ");
        }
        doc.append("bits=\""+bitsList+"\" />\n");
        if(!bitsList.isEmpty() ) doc.append("\n");
    }
    doc.append("</parts>\n\n");
    fileName.replace(".dat","_regs-aut.xml");
    QFile file( fileName );

    if( !file.open( QFile::WriteOnly | QFile::Text ))
    {
        QApplication::restoreOverrideCursor();
        MessageBoxNB( "McuCreator::convert",
        "Cannot write file:\n+."+fileName+file.errorString() );
        return ;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc;
    file.close();
}

