/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shiftreg.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

#define tr(str) simulideTr("ShiftReg",str)

Component* ShiftReg::construct( QString type, QString id )
{ return new ShiftReg( type, id ); }

LibraryItem* ShiftReg::libraryItem()
{
    return new LibraryItem(
        tr( "Shift Register" ),
        "Arithmetic",
        "1to3.png",
        "ShiftReg",
        ShiftReg::construct );
}

ShiftReg::ShiftReg( QString type, QString id )
        : LogicComponent( type, id )
{
    m_width  = 4;
    m_height = 9;

    init({         // Inputs:
            "IL03DI",
            "IL05>",
            "IL07Rst"
        });

    setNumOuts( 8, "Q" );
    createOePin( "IU01OE ", id+"-in3");

    m_clkPin = m_inPin[1];

    setResetInv( true );         // Invert Reset Pin

    addPropGroup( { tr("Main"), {
new BoolProp<ShiftReg>("Clock_Inverted", tr("Clock Inverted"),"", this, &ShiftReg::clockInv, &ShiftReg::setClockInv ),
new BoolProp<ShiftReg>("Reset_Inverted", tr("Reset Inverted"),"", this, &ShiftReg::resetInv, &ShiftReg::setResetInv ),
    }, groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Timing")  , IoComponent::edgeProps(),0 } );
}
ShiftReg::~ShiftReg(){}

void ShiftReg::stamp()
{
    m_clkPin->changeCallBack( this );
    m_inPin[2]->changeCallBack( this ); // Reset
    LogicComponent::stamp();
}

void ShiftReg::voltChanged()
{
    updateOutEnabled();
    updateClock();

    bool clkRising = (m_clkState == Clock_Rising);// Get Clk to don't miss any clock changes
    bool     reset = m_inPin[2]->getInpState();

    if( reset ) m_nextOutVal = 0;        // Reset shift register
    else if( clkRising )                 // Clock rising edge
    {
        m_nextOutVal <<= 1;

        bool data = m_inPin[0]->getInpState();
        if( data ) m_nextOutVal += 1;
    }
    scheduleOutPuts( this );
}

void ShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_inPin[2]->setInverted( inv );
}
