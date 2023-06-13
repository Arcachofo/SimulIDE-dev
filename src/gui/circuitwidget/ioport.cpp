/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>
#include <math.h>

#include "ioport.h"

IoPort::IoPort( QString name )
{
    m_name = name;
    m_shortName = "P"+ name.right(1);
    m_numPins = 0;
}
IoPort::~IoPort(){}

void IoPort::reset()
{
    m_pinState = 0;
    m_pinDirection = 0;

    for( IoPin* pin : m_pins ) {
        pin->setOutState( 0 );
        pin->setPinMode( input );
    }
}

void IoPort::setOutState( uint32_t val )
{
    uint32_t changed = m_pinState ^ val; // See which Pins have actually changed
    if( changed == 0 ) return;
    m_pinState = val;

    for( int i=0; i<m_numPins; ++i ){
        uint32_t flag = 1<<i;
        if( changed & flag ) m_pins[i]->setOutState( (val & flag) > 0 ); // Pin changed
    }
}

void IoPort::setOutStatFast( uint32_t val )
{
    uint32_t changed = m_pinState ^ val; // See which Pins have actually changed
    if( changed == 0 ) return;
    m_pinState = val;

    for( int i=0; i<m_numPins; ++i ){
        uint32_t flag = 1<<i;
        if( changed & flag ) m_pins[i]->setOutStatFast( (val & flag) > 0 ); // Pin changed
    }
}

uint32_t IoPort::getInpState()
{
    uint32_t data = 0;
    for( int i=0; i<m_numPins; ++i )
        if( m_pins[i]->getInpState() ) data += (1 << i);
    return data;
}

void IoPort::setDirection( uint32_t val )
{
    uint32_t changed = m_pinDirection ^ val;  // See which Pins have actually changed
    if( changed == 0 ) return;
    m_pinDirection = val;

    for( int i=0; i<m_numPins; ++i ){
        uint32_t flag = 1<<i;
        if( changed & flag ) m_pins[i]->setPinMode( ((val & flag) > 0) ? output : input ); // Pin changed
}   }

void IoPort::setPinMode( pinMode_t mode )
{
    for( IoPin* pin : m_pins ) pin->setPinMode( mode );
}

void IoPort::changeCallBack( eElement* el, bool ch )
{
    for( IoPin* pin : m_pins ) pin->changeCallBack( el, ch );
}

void IoPort::createPins( Component* comp, QString pins, uint32_t pinMask )
{
    m_numPins = pins.toUInt(0,0);
    if( m_numPins )
    {
        m_pins.resize( m_numPins );

        for( int i=0; i<m_numPins; ++i )
        {
            if( pinMask & 1<<i )
                m_pins[i] = createPin( i, m_name+QString::number(i) , comp );//new IoPin( this, i, m_name+QString::number(i), IoComp );
        }
    }else{
        QStringList pinList = pins.split(",");
        for( QString pinName : pinList )
        {
            IoPin* pin = createPin( m_numPins, m_name+pinName , comp );//new IoPin( this, i, m_name+pinName, IoComp );
            m_pins.emplace_back( pin );
            m_numPins++;
        }
    }
}

IoPin* IoPort::createPin( int i, QString id, Component* comp )
{
    IoPin* pin = new IoPin( 0, QPoint(0,0), comp->getUid()+"-"+id, i, comp, input );
    pin->setOutHighV( 5 );
    pin->setPinState( input_low );
    return pin;
}

IoPin* IoPort::getPinN( uint8_t i )
{
    if( i >= m_pins.size() ) return NULL;
    return m_pins[i];
}

IoPin* IoPort::getPin( QString pinName )
{
    IoPin* pin = NULL;
    if( pinName.startsWith( m_name ) || pinName.startsWith( m_shortName ) )
    {
        QString pinId = pinName.remove( m_name ).remove( m_shortName );
        int pinNumber = pinId.toInt();
        pin = getPinN( pinNumber );
    }else{
        for( IoPin* ioPin : m_pins )
        {
            QString pid = ioPin->pinId();
            pid = pid.split("-").last().remove( m_name );
            if( pid == pinName )
                return ioPin;
        }
    }
    //if( !pin )
    //    qDebug() << "ERROR: IoPort::getPin NULL Pin:"<< pinName;
    return pin;
}

// ---- Script Engine -------------------
#include "angelscript.h"
void IoPort::registerScript( asIScriptEngine* engine )
{
    engine->RegisterObjectType("IoPort", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("IoPort", "void setPinMode(uint m)"
                                   , asMETHODPR( IoPort, setPinMode, (uint), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPort", "uint getInpState()"
                                   , asMETHODPR( IoPort, getInpState, (), uint)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPort", "void setOutState(uint s)"
                                   , asMETHODPR( IoPort, setOutState, (uint), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPort", "void changeCallBack(eElement@ s, bool s)"
                                   , asMETHODPR( IoPort, changeCallBack, (eElement*, bool), void)
                                   , asCALL_THISCALL );
}
