/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrinterrupt.h"
#include "e_mcu.h"
#include "datautils.h"

Interrupt* AVRInterrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu ) // Static
{
    return new AVRInterrupt( name, vector, mcu );
}

AVRInterrupt::AVRInterrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
    //m_I = getRegBits( "I", mcu );

    //m_autoClear = true;
    //m_remember  = true;
}
AVRInterrupt::~AVRInterrupt(){}

/*void AVRInterrupt::execute()
{
    clearRegBits( m_I );             // Clear SREG I flag
    m_interrupts->enableGlobal( 0 ); // Disable interrupts
    Interrupt::execute();
}

void AVRInterrupt::exitInt() // Exit from this interrupt
{
    setRegBits( m_I );                // Set SREG I flag
    m_interrupts->enableGlobal( 2 );  // Enable interrupts (execute next cycle)
    Interrupt::exitInt();
}*/

