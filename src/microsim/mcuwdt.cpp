/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuwdt.h"
#include "e_mcu.h"
#include "cpubase.h"
#include "mcuinterrupts.h"
#include "simulator.h"

McuWdt::McuWdt( eMcu* mcu, QString name )
      : McuPrescaled( mcu, name )
      , eElement( mcu->getId()+"-"+name )
{
    m_wdtFuse  = false;
}
McuWdt::~McuWdt(){}

void McuWdt::initialize()
{
    m_ovfInter = false;
    m_ovfReset = false;
}

void McuWdt::runEvent()            // Overflow
{
    if( !m_wdtFuse ) return;

    if( m_ovfInter ) m_interrupt->raise();
    if( m_ovfReset )
    {
        qDebug() << "McuWdt::runEvent - Watchdog Reset\n";
        m_mcu->cpu->reset();
    }
    Simulator::self()->addEvent( m_ovfPeriod, this );
}

