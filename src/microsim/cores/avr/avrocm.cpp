/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrocm.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrOcm::AvrOcm( eMcu* mcu, QString name )
      : McuOcm( mcu, name )
{
}
AvrOcm::~AvrOcm(){}

void AvrOcm::configureA( uint8_t newVal )
{
    m_mode = newVal;
}

void AvrOcm::OutputOcm() //Set Ocm output from OCnB1 & OCnB2
{
    if( m_mode ) m_oPin->sheduleState( m_state1 || m_state2, 0 );
    else         m_oPin->sheduleState( m_state1 && m_state2, 0 );
}
