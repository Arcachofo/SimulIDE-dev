/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrcomparator.h"
#include "datautils.h"
#include "regwatcher.h"
#include "e_mcu.h"
#include "mcupin.h"

AvrComp::AvrComp( eMcu* mcu, QString name )
       : McuComp( mcu, name )
{
    m_ACD  = getRegBits( "ACD", mcu );
    m_ACBG = getRegBits( "ACBG", mcu );
    m_ACO  = getRegBits( "ACO", mcu );
    m_ACI  = getRegBits( "ACI", mcu );
    m_ACIC = getRegBits( "ACIC", mcu );
    m_ACIS = getRegBits( "ACIS0,ACIS1", mcu );

    m_AIN0D = getRegBits( "AIN0D", mcu );
    m_AIN1D = getRegBits( "AIN1D", mcu );

    watchBitNames( "ACO", R_READ, this, &AvrComp::compare, mcu );
}
AvrComp::~AvrComp(){}

void AvrComp::initialize()
{
    m_pinP = m_pins[0];
    m_pinN = m_pins[1];
}

void AvrComp::voltChanged()
{
    compare(0);
}

void AvrComp::configureA( uint8_t newACSR ) // ACSR is being written
{
    if( newACSR & m_ACI.mask )
        m_mcu->m_regOverride = newACSR & ~(m_ACI.mask); // Clear ACI by writting it to 1

    m_enabled = !getRegBitsBool( newACSR, m_ACD );
    if( !m_enabled ) m_mcu->m_regOverride = newACSR & ~m_ACO.mask; // Clear ACO

    m_fixVref = getRegBitsVal( newACSR, m_ACBG );

    /// TODO: ACIC: Analog Comparator Input Capture Enable

    m_mode = getRegBitsVal( newACSR, m_ACIS );
}

void AvrComp::configureB( uint8_t newAIND ) // AIN0D,AIN1D being written
{
    /// TODO: Disable Digital Input buffer.
    /// The corresponding PIN Register bit will always read as zero when this bit is set
}

void AvrComp::configureC( uint8_t newACOE )
{
    if( newACOE ) m_pinOut = m_pins[2];
    else          m_pinOut = NULL;
    m_pins[2]->controlPin( newACOE, newACOE );

    m_pins[0]->changeCallBack( m_pins[0], newACOE );
    m_pins[1]->changeCallBack( m_pins[1], newACOE );
}

void AvrComp::compare( uint8_t ) // Performed only when ACO is readed
{
    if( !m_enabled ) return;

    double vRef = m_fixVref ? 1.1 : m_pinP->getVoltage();
    bool compOut = vRef > m_pinN->getVoltage() ;
    bool rising = !m_compOut && compOut;

    if( m_compOut != compOut )
    {
        if( compOut ) setRegBits( m_ACO );
        else          clearRegBits( m_ACO );

        switch( m_mode ){
            case 0: m_interrupt->raise();               break; // Comparator Interrupt on Output Toggle.
            case 1:                                     break; // Reserved
            case 2: if( !rising ) m_interrupt->raise(); break; // Comparator Interrupt on Falling Output Edge.
            case 3: if(  rising ) m_interrupt->raise();        // Comparator Interrupt on Rising  Output Edge.
        }
        m_compOut = compOut;
        if( m_pinOut ) m_pinOut->sheduleState( compOut, 0 );
    }
}
