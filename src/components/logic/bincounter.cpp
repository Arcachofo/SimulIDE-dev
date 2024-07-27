/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "bincounter.h"
#include "itemlibrary.h"
#include "connector.h"
#include "iopin.h"

#include "intprop.h"
#include "boolprop.h"

#define tr(str) simulideTr("BinCounter",str)

Component *BinCounter::construct( QString type, QString id)
{ return new BinCounter( type, id); }

LibraryItem* BinCounter::libraryItem()
{
    return new LibraryItem(
        tr("Binary Counter"),
        "Arithmetic",
        "2to1.png",
        "Counter",
        BinCounter::construct );
}

BinCounter::BinCounter( QString type, QString id)
          : LogicComponent( type, id )
{
    m_TopValue = 1;
    m_width  = 3;
    m_height = 3;

    init({         // Inputs:
            "IL01>",
            "IL02R",
            "IU01S",
                   // Outputs:
            "OR01Q"
        });

    m_clkPin = m_inPin[0];     // eClockedDevice
    m_rstPin = m_inPin[1];
    m_setPin = m_inPin[2];

    setSrInv( true );            // Invert Reset Pin
    useSetPin( false );          // Don't use Set Pin

    addPropGroup( { tr("Main"), {
        new BoolProp<BinCounter>("Pin_SET", tr("Use Set Pin"),""
                                , this, &BinCounter::pinSet,&BinCounter::useSetPin, propNoCopy ),

        new BoolProp<BinCounter>("Clock_Inverted", tr("Clock Inverted"),""
                                , this, &BinCounter::clockInv, &BinCounter::setClockInv ),

        new BoolProp<BinCounter>("Reset_Inverted", tr("Set/Reset Inverted"),""
                                , this, &BinCounter::srInv, &BinCounter::setSrInv ),

        new IntProp <BinCounter>("Max_Value", tr("Count to"),""
                                , this, &BinCounter::maxVal, &BinCounter::setMaxVal,0,"uint" ),
    },groupNoCopy} );

    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps(),0 } );
    addPropGroup( { tr("Timing")  , IoComponent::edgeProps(),0 } );
}
BinCounter::~BinCounter(){}

void BinCounter::stamp()
{
    m_Counter = 0;
    m_rstPin->changeCallBack( this );
    m_setPin->changeCallBack( this );
    LogicComponent::stamp();
}

void BinCounter::voltChanged()
{
    updateClock();
    bool clkRising = ( m_clkState == Clock_Rising );

    if( m_rstPin->getInpState() ) // Reset
    {
       m_Counter = 0;
       m_nextOutVal = 0;
    }
    else if( m_pinSet && m_setPin->getInpState() ) // Set
    {
       m_Counter = m_TopValue;
       m_nextOutVal = 1;
    }
    else if( clkRising )
    {
        m_Counter++;

        if(      m_Counter == m_TopValue ) m_nextOutVal = 1;
        else if( m_Counter >  m_TopValue )
        {
            m_Counter = 0;
            m_nextOutVal = 0;
    }   }
    IoComponent::scheduleOutPuts( this );
}

void BinCounter::setSrInv( bool inv )
{
    m_resetInv = inv;
    m_rstPin->setInverted( inv );

    if( m_pinSet ) m_setPin->setInverted( inv );
    else           m_setPin->setInverted( false );
}

void BinCounter::useSetPin( bool set )
{
    m_pinSet = set;
    if( !set ) m_setPin->removeConnector();
    m_setPin->setVisible( set );
}
