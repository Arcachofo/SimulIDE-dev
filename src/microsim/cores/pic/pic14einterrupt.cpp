/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "pic14einterrupt.h"
#include "e_mcu.h"
#include "datautils.h"
#include "pic14ecore.h"

Pic14eInterrupt::Pic14eInterrupt( QString name, uint16_t vector, eMcu* mcu )
               : PicInterrupt( name, vector, mcu )
{
}
Pic14eInterrupt::~Pic14eInterrupt(){}

void Pic14eInterrupt::execute() // Save context
{
    Pic14eCore* picCpu = static_cast<Pic14eCore*>(m_mcu->cpu());
    picCpu->saveContext();

    Interrupt::execute();
}

void Pic14eInterrupt::exitInt() // Restore context
{
    Pic14eCore* picCpu = static_cast<Pic14eCore*>(m_mcu->cpu());
    picCpu->restoreContext();

    Interrupt::exitInt();
}

