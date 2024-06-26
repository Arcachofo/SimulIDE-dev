/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shiftreg.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

#include "intprop.h"
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
    m_parallelIn = false;
    m_ldInps = false;

    m_dinPin = new IoPin( 0, QPoint(0,0), id+"-in0", 0, this, input ); // Will be configured later
    m_clkPin = new IoPin( 0, QPoint(0,0), id+"-in1", 0, this, input );
    m_rstPin = new IoPin( 0, QPoint(0,0), id+"-in2", 0, this, input );
    m_serPin = new IoPin( 0, QPoint(0,0), id+"-in4", 0, this, input );
    m_serPin->setInverted( true );

    createOePin("IU02OE ", id+"-in3"); //createOePin( "IU01OE ", id+"-in3");
    setBits( 8 );
    setupPin( m_dinPin, "L03DI" );
    setupPin( m_clkPin, "L05>" );
    setupPin( m_rstPin, "L07Rst" );
    setupPin( m_serPin, "L00S" );

    setResetInv( true );         // Invert Reset Pin

    addPropGroup( { tr("Main"), {
        new IntProp <ShiftReg>("Bits", tr("Size"),"_bits"
                            , this, &ShiftReg::bits, &ShiftReg::setBits, propNoCopy,"uint" ),

        new BoolProp<ShiftReg>("Parallel_input", tr("Parallel Input"),""
                              , this, &ShiftReg::parallelIn, &ShiftReg::setParallelIn, propNoCopy ),

        new BoolProp<ShiftReg>("Clock_Inverted", tr("Clock Inverted"),""
                              , this, &ShiftReg::clockInv, &ShiftReg::setClockInv ),

        new BoolProp<ShiftReg>("Reset_Inverted", tr("Reset Inverted"),""
                              , this, &ShiftReg::resetInv, &ShiftReg::setResetInv ),
    }, groupNoCopy} );

    addPropGroup( { tr("Electric"), IoComponent::inputProps()
                                  + IoComponent::outputProps()
                                  + IoComponent::outputType(),0 } );

    addPropGroup( { tr("Timing")  , IoComponent::edgeProps(),0 } );
}
ShiftReg::~ShiftReg(){}

void ShiftReg::stamp()
{
    m_clkPin->changeCallBack( this );
    m_rstPin->changeCallBack( this ); // Reset
    m_serPin->changeCallBack( this, m_parallelIn ); // Serial/Parallel changes
    LogicComponent::stamp();
}

void ShiftReg::voltChanged()
{
    updateOutEnabled();
    updateClock();

    bool clkRising = (m_clkState == Clock_Rising);// Get Clk to don't miss any clock changes
    bool     reset = m_rstPin->getInpState();

    bool ldInps = false;
    if( m_parallelIn )
    {
        ldInps = !m_serPin->getInpState(); // High = load parallel

        if( m_ldInps != ldInps ){
            m_ldInps = ldInps;
            for( IoPin* pin : m_inPin )
                pin->changeCallBack( this, ldInps );
        }
    }

    if( reset ) m_nextOutVal = 0;        // Reset shift register
    else if( m_parallelIn && m_ldInps )  // Load parallel
    {
        m_nextOutVal = 0;
        for( uint i=0; i<m_inPin.size(); ++i )
            if( m_inPin[i]->getInpState() ) m_nextOutVal |= 1<<i;
    }
    else if( clkRising )                 // Shift
    {
        m_nextOutVal <<= 1;

        bool data = m_dinPin->getInpState();
        if( data ) m_nextOutVal += 1;
    }
    scheduleOutPuts( this );
}

void ShiftReg::setBits( int b )
{
    if( b < 1 ) b = 1;
    m_bits = b;
    updatePins();
}

void ShiftReg::setParallelIn( bool p )
{
    m_parallelIn = p;
    updatePins();
    if( Simulator::self()->isRunning() ) return; // No changes

    m_serPin->setVisible( p );

    if( p ) voltChanged();
    else{
        for( IoPin* pin : m_inPin )
            pin->changeCallBack( this, false );
    }
}

void ShiftReg::updatePins()
{
    int inBits = m_parallelIn ? m_bits : 0;
    setNumOuts( m_bits, "Q" );
    setNumInps( inBits, "I", 0, 10 ); // Start at id 10 to leave room for new control inputs
    setNumOuts( m_bits, "Q" ); // Fix pin positions

    int start = inBits-m_height/2;
    if( m_parallelIn ) start++;
    m_serPin->setY( 8*(0+start) );
    m_dinPin->setY( 8*(1+start) );
    m_clkPin->setY( 8*(2+start) );
    m_rstPin->setY( 8*(3+start) );

    int noPara = (m_bits < 3) ? 3-m_bits : 0;
    int deltaH = m_parallelIn ? 4 : noPara;
    m_area.setHeight(  m_area.height()+deltaH*8 );

    setupPin( m_oePin, "U02OE" ); // Reposition OE pin
}

void ShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_rstPin->setInverted( inv );
}

void ShiftReg::remove()
{
    m_serPin->removeConnector();
    m_dinPin->removeConnector();
    m_clkPin->removeConnector();
    m_rstPin->removeConnector();
    LogicComponent::remove();
}
