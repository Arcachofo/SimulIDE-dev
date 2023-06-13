/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "mcuport.h"
//#include "mcupin.h"
#include "mcu.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "datautils.h"

McuPort::McuPort( eMcu* mcu, QString name )
       : McuModule( mcu, name )
{
    m_numPins = 0;

    m_shortName = "P"+ name.right(1);

    m_outReg = NULL;
    m_dirReg = NULL;
    m_inReg  = NULL;

    m_intMask = 0;
    m_outAddr = 0;
    m_inAddr  = 0;
    m_dirAddr = 0;
    m_dirInv  = false;
    m_rstIntMask = true;
}
McuPort::~McuPort(){}

void McuPort::reset()
{
    m_pinState = 0;
    if( m_rstIntMask ) m_intMask = 0;
    /// for( McuPin* pin : m_pins ) pin->reset();
}

void McuPort::pinChanged( uint8_t pinMask, uint8_t val ) // Pin number in pinMask
{
    uint8_t pinState = (m_pinState & ~pinMask) | (val & pinMask);

    if( pinState == m_pinState ) return;
    m_pinState = pinState;

    if( m_intMask & pinMask ) m_interrupt->raise(); // Pin change interrupt

    if( m_inAddr ) *m_inReg = m_pinState; //m_mcu->writeReg( m_inAddr, m_pinState, false ); // Write to RAM
}

void McuPort::outChanged( uint8_t val )
{
    uint8_t changed = *m_outReg ^ val; // See which Pins have actually changed
    if( changed == 0 ) return;
    *m_outReg = val;

    /*uint8_t outputs = 0xFF;
    if( m_dirReg )
    {
        if( m_dirInv ) outputs = ~(*m_dirReg); // defaul: 1 for outputs, inverted: 0 for outputs (PICs)
        else           outputs =   *m_dirReg;
    }
    uint8_t pinCh = outputs & changed;
    if( pinCh ) pinChanged( pinCh, val ); // Update Pin states*/

    for( int i=0; i<m_numPins; ++i ){
        if( changed & (1<<i) ) m_pins[i]->setPortState( val & (1<<i) ); // Pin changed
}   }

void McuPort::dirChanged( uint8_t val )
{
    uint8_t changed = *m_dirReg ^ val;  // See which Pins have actually changed
    if( changed == 0 ) return;

    if( m_dirInv ) val = ~val;   // defaul: 1 for outputs, inverted: 0 for outputs (PICs)

    for( int i=0; i<m_numPins; ++i ){
        if( changed & 1<<i) m_pins[i]->setDirection( val & (1<<i) ); // Pin changed
}   }

void McuPort::readPort( uint8_t )
{
    if( m_inAddr ) *m_inReg = getInpState();
}

void McuPort::intChanged( uint8_t val )
{
    if( m_intBits.reg ) m_intMask = getRegBitsVal( val, m_intBits );
    else                m_intMask = val;
}
// Direct Control---------------------------------------------------

void McuPort::controlPort( bool outCtrl, bool dirCtrl )
{
    for( int i=0; i<m_numPins; ++i )
        m_pins[i]->controlPin( outCtrl, dirCtrl );
}

void McuPort::setDirection( uint val )
{
    for( int i=0; i<m_numPins; ++i )
        m_pins[i]->setDirection( val & (1<<i) );
}

void McuPort::setOutState( uint val )
{
    for( int i=0; i<m_numPins; ++i )
    {
        bool state = (val & 1<<i) > 0;
        if( state != m_pins[i]->getOutState() )
            m_pins[i]->setOutState( state );
    }
}

uint McuPort::getInpState()
{
    uint data = 0;
    for( int i=0; i<m_numPins; ++i )
        if( m_pins[i]->getInpState() ) data += (1 << i);
    return data;
}

void McuPort::setPinMode( pinMode_t mode )
{
    for( int i=0; i<m_numPins; ++i ) m_pins[i]->setPinMode( mode );
}
//-------------------------------------------------------------------

void McuPort::setPullups( uint8_t puMask )
{
    for( int i=0; i<m_numPins; ++i )
        m_pins[i]->setPullup( puMask & 1<<i );
}

void McuPort::setAllPullups( uint8_t val )
{
    uint8_t puMask = val ? 255 : 0;
    setPullups( puMask );
}

void McuPort::clearAllPullups( uint8_t val )
{
    uint8_t puMask = val ? 0 : 255;
    setPullups( puMask );
}

void McuPort::createPins( Mcu* mcuComp, QString pins, uint32_t pinMask )
{
    m_numPins = pins.toUInt(0,0);
    if( m_numPins )
    {
        m_pins.resize( m_numPins );

        for( int i=0; i<m_numPins; ++i )
        {
            if( pinMask & 1<<i )
                m_pins[i] = createPin( i, mcuComp->getUid()+"-"+m_name+QString::number(i) , mcuComp );//new McuPin( this, i, m_name+QString::number(i), mcuComp );
        }
    }else{
        QStringList pinList = pins.split(",");
        pinList.removeAll("");
        for( QString pinName : pinList )
        {
            McuPin* pin = createPin( m_numPins, mcuComp->getUid()+"-"+m_name+pinName , mcuComp );//new McuPin( this, i, m_name+pinName, mcuComp );
            m_pins.emplace_back( pin );
            m_numPins++;
        }
    }
}

McuPin* McuPort::createPin( int i, QString id , Component* mcu )
{
    return new McuPin( this, i, id, mcu );
}

McuPin* McuPort::getPinN( uint8_t i )
{
    if( i >= m_pins.size() ) return NULL;
    return m_pins[i];
}

McuPin* McuPort::getPin( QString pinName )
{
    McuPin* pin = NULL;

    if( pinName.startsWith( m_name ) || pinName.startsWith( m_shortName ) )
    {
        QString pinId = pinName.remove( m_name ).remove( m_shortName );
        int pinNumber = pinId.toInt();
        pin = getPinN( pinNumber );
    }else{
        for( McuPin* mcuPin : m_pins )
        {
            if( !mcuPin ) continue;
            QString pid = mcuPin->pinId();
            pid = pid.split("-").last().remove( m_name );
            if( pid == pinName ) return mcuPin;
        }
    }
    //if( !pin )
    //    qDebug() << "ERROR: McuPort::getPin NULL Pin:"<< pinName;
    return pin;
}

// ---- Script Engine -------------------
#include "angelscript.h"
void McuPort::registerScript( asIScriptEngine* engine )
{
    engine->RegisterObjectType("McuPort", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("McuPort", "void controlPort( bool o, bool d )"
                                   , asMETHODPR( McuPort, controlPort, (bool, bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPort", "void setDirection( uint d )"
                                   , asMETHODPR( McuPort, setDirection, (uint), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPort", "uint getInpState()"
                                   , asMETHODPR( McuPort, getInpState, (), uint)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPort", "void setOutState(uint s)"
                                   , asMETHODPR( McuPort, setOutState, (uint), void)
                                   , asCALL_THISCALL );
}
