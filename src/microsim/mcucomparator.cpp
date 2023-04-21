/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcucomparator.h"
#include "e_mcu.h"

McuComp::McuComp( eMcu* mcu, QString name )
       : McuModule( mcu, name )
       , eElement( mcu->getId()+"-"+name )
{
    m_pinP = NULL;
    m_pinN = NULL;
    m_pinOut = NULL;
    m_enabled = false;
}
McuComp::~McuComp(){}

void McuComp::initialize()
{
    m_fixVref = false;
    m_enabled = true;
    m_compOut = false;
    /// m_vref = 0; // Let Vref Module set this value.
    setMode( 0 );
}

void McuComp::callBackDoub( double vref ) // Called from Vref module
{
    m_vref = vref;
    if( m_enabled ) voltChanged();
}
