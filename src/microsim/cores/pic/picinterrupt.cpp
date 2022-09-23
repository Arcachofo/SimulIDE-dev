/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picinterrupt.h"
#include "e_mcu.h"
#include "datautils.h"

PicInterrupt::PicInterrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
    //m_GIE = getRegBits( "GIE", mcu );

    m_autoClear = false;
    //m_remember  = true;
}
PicInterrupt::~PicInterrupt(){}

/*void PicInterrupt::execute()
{
    clearRegBits( m_GIE );
    m_interrupts->enableGlobal( 0 ); // Disable interrupts
    Interrupt::execute();
}

void PicInterrupt::exitInt()
{
    setRegBits( m_GIE );
    m_interrupts->enableGlobal( 1 );  // Enable interrupts
    Interrupt::exitInt();
}*/

