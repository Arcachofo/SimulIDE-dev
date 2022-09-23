/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51interrupt.h"
#include "i51usart.h"

I51Interrupt::I51Interrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
}
I51Interrupt::~I51Interrupt(){}

// Static --------------------------
Interrupt* I51Interrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu )
{
    //if     ( name.startsWith( "EXT") ) return new I51ExtInt( name, vector, mcu );
    //else
        if( name == "T1_OVF"        ) return new I51T1Int( name, vector, mcu );

    return new Interrupt( name, vector, mcu );
}

//_________________________________________________________________
//_________________________________________________________________

/*I51ExtInt::I51ExtInt( QString name, uint16_t vector, eMcu* mcu )
         : Interrupt( name, vector, mcu )
{}
I51ExtInt::~I51ExtInt(){}

void I51ExtInt::reset()
{
    m_lastValue = 0;
    Interrupt::reset();
}

void I51ExtInt::raise( uint8_t v )
{
    if( !m_enabled ) return;

    if( m_mode == 0 )
    {
        if( v == 0 ) Interrupt::raise( v ); // Low  state: raise
        else         m_raised = false;      // High state: stop triggering
    }
    else if( m_mode == 1 )
    {
        if( (v==0) && m_lastValue ) Interrupt::raise( v ); // Falling edge
    }
    m_lastValue = v;
}

void I51ExtInt::exitInt() // Exit from this interrupt
{
    Interrupt::exitInt();

    if( m_mode == 0 ) //In mode 0 keep triggering until pin change state
    {
        if( m_raised ) m_interrupts->addToPending( this ); // Add to pending interrupts
        else           m_interrupts->remFromPending( this );
    }
    else if( m_mode == 1 ) Interrupt::clearFlag();
}*/

//_________________________________________________________________
//_________________________________________________________________

I51T1Int::I51T1Int( QString name, uint16_t vector, eMcu* mcu )
        : Interrupt( name, vector, mcu )
{}
I51T1Int::~I51T1Int(){}

void I51T1Int::reset()
{
    m_usart = NULL;
    Interrupt::reset();
}

void I51T1Int::raise( uint8_t v )
{
    //if( !m_enabled ) return;

    if( m_usart ) m_usart->step();
    Interrupt::raise( v );
}
