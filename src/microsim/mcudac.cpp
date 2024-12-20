/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcudac.h"
#include "e_mcu.h"

McuDac::McuDac( eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , eElement( mcu->getId()+"-"+name )
{
    m_dacReg = nullptr;

    m_pRefPin = nullptr;
    m_nRefPin = nullptr;
    m_outPin  = nullptr;
}
McuDac::~McuDac(){}

void McuDac::initialize()
{
    m_enabled   = false;
    m_outVoltEn = false;

    m_outVal = 0;
    m_vRefP = 0;
    m_vRefN = 0;
}
