/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "bincounter.h"
#include "itemlibrary.h"
#include "simulator.h"
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
        "2to3g.png",
        "BinCounter",
        BinCounter::construct );
}

BinCounter::BinCounter( QString type, QString id)
          : LogicComponent( type, id )
{
    m_topValue = 15;
    m_width  = 4;
    m_height = 6;
    m_area = QRect(-m_width*4,-m_height*4, m_width*8, m_height*8 );

    m_clkPin = new IoPin( 0, QPoint(0,0), id+"-Pin_clk", 0, this, input );
    m_dirPin = new IoPin( 0, QPoint(0,0), id+"-Pin_dir", 0, this, input );
    m_rstPin = new IoPin( 0, QPoint(0,0), id+"-Pin_rst", 0, this, input );
    m_ldPin  = new IoPin( 0, QPoint(0,0), id+"-Pin_ldp", 0, this, input );
    m_rcoPin = new IoPin( 0, QPoint(0,0), id+"-Pin_rco", 0, this, output );
    m_rboPin = new IoPin( 0, QPoint(0,0), id+"-Pin_rbo", 0, this, output );

    m_otherPin.emplace_back( m_clkPin );
    m_otherPin.emplace_back( m_dirPin );
    m_otherPin.emplace_back( m_rstPin );
    m_otherPin.emplace_back( m_ldPin );
    m_otherPin.emplace_back( m_rcoPin );
    m_otherPin.emplace_back( m_rboPin );

    setupPin( m_clkPin,"L05>"  );
    setupPin( m_dirPin,"L06Dir");
    setupPin( m_rstPin,"L07Rst");
    setupPin( m_ldPin ,"L00LD" );
    setupPin( m_rcoPin,"R06CO" );
    setupPin( m_rboPin,"R07BO" );

    m_ldPin->setVisible( false );
    m_dirPin->setVisible( false );
    m_rcoPin->setVisible( false );
    m_rboPin->setVisible( false );
    m_bidirectional = false;
    m_parallelIn = false;
    m_useRCO = false;
    setBits( 4 );

    m_dirPin->setInverted( true );
    m_rstPin->setInverted( true ); //setSrInv( true );            // Invert Reset Pin

    addPropGroup( { tr("Main"), {

        new IntProp <BinCounter>("Bits", tr("Size"),"_bits"
                               , this, &BinCounter::bits, &BinCounter::setBits, propNoCopy,"uint" ),

        new IntProp <BinCounter>("Max_Value", tr("Top Value"),""
                               , this, &BinCounter::maxVal, &BinCounter::setMaxVal,0,"uint" ),

        new BoolProp<BinCounter>("Parallel_input", tr("Parallel Input"),""
                               , this, &BinCounter::parallelIn, &BinCounter::setParallelIn, propNoCopy ),

        new BoolProp<BinCounter>("RCO", tr("Use RCO & RBO"),""
                               , this, &BinCounter::useRCO, &BinCounter::setUseRCO, propNoCopy ),

        new BoolProp<BinCounter>("Bidir", tr("Bidirectional"),""
                                , this, &BinCounter::bidirectional, &BinCounter::setBidirectional, propNoCopy ),

        //new BoolProp<BinCounter>("Clock_Inverted", tr("Clock Inverted"),""
        //                        , this, &BinCounter::clockInv, &BinCounter::setClockInv ),

        //new BoolProp<BinCounter>("Reset_Inverted", tr("Reset Inverted"),""
        //                        , this, &BinCounter::srInv, &BinCounter::setSrInv ),
    },groupNoCopy} );

    appendPropGroup( tr("Main"), IoComponent::familyProps() );

    addPropGroup( { tr("Inputs"),
          IoComponent::inputProps()
        + QList<ComProperty*>({
          new BoolProp<BinCounter>("Invert_Inputs", tr("Invert Inputs"),""
                                  , this, &BinCounter::invertInps, &BinCounter::setInvertInps,propNoCopy )
          })
    ,0 } );

    addPropGroup( { tr("Outputs"),
          IoComponent::outputProps()
        + IoComponent::outputType(),0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
BinCounter::~BinCounter(){}

bool BinCounter::propNotFound( QString prop, QString val )
{
    if( prop =="Clock_Inverted" ) // Old circuits
    {
        m_clkPin->setInverted( val == "true" );
    }
    else if( prop =="Reset_Inverted" ) // Old circuits
    {
        bool invert = (val == "true");
        m_rstPin->setInverted( invert );
    }
    else return false;

    return true;
}

void BinCounter::stamp()
{
    m_counter = 0;

    m_rcoPin->setOutState( false );
    m_rboPin->setOutState( false );

    m_rstPin->changeCallBack( this );
    m_ldPin->changeCallBack( this );
    m_clkPin->changeCallBack( this );

    LogicComponent::stamp();
}

void BinCounter::voltChanged()
{
    updateClock();
    bool clkRising = ( m_clkState == Clock_Rising );

    if( m_rstPin->getInpState() ) // Reset
    {
       m_counter = 0;
       m_nextOutVal = 0;
    }
    else if( m_parallelIn && m_ldPin->getInpState() ) // Load
    {
        m_counter = 0;
        for( uint i=0; i<m_inpPin.size(); ++i )
            if( m_inpPin[i]->getInpState() ) m_counter |= 1<<i;

        m_nextOutVal = m_counter;
    }
    else if( clkRising )              // Clock edge
    {
        if( m_dirPin->getInpState() ) // Count up
        {
            m_counter++;
            if     ( m_counter == m_topValue ) m_rcoPin->scheduleState( true, m_delayBase*m_delayMult );
            else if( m_counter >  m_topValue )
            {
                m_counter = 0;
                m_rcoPin->scheduleState( false, m_delayBase*m_delayMult );
            }
        }else{                        // Count down
            m_counter--;
            if     ( m_counter == 0 ) m_rboPin->scheduleState( true, m_delayBase*m_delayMult );
            else if( m_counter < 0 )
            {
                m_counter = m_topValue;
                m_rboPin->scheduleState( false, m_delayBase*m_delayMult );
            }
        }
        m_nextOutVal = m_counter;
    }
    //qDebug() << "BinCounter::voltChanged" << m_counter;
    IoComponent::scheduleOutPuts( this );
}

void BinCounter::updatePins()
{
    int inBits = m_parallelIn ? m_bits : 0;
    setNumOuts( m_bits, "Q" );
    setNumInps( inBits, "I", 0, 10 ); // Start at id 10 to leave room for new control inputs
    updtOutPins(); // Fix pin positions

    int start = inBits-m_height/2;
    if( m_parallelIn ) { start++; inBits++; }
    m_ldPin->setY( 8*(start++) );
    if( m_bidirectional ) m_dirPin->setY( 8*(start++) );
    m_clkPin->setY( 8*(start++) );
    m_rstPin->setY( 8*(start++) );

    if( m_useRCO )
    {
        m_rcoPin->setY( m_area.y() + 8*(m_bits+2) );
        m_rboPin->setY( m_area.y() + 8*(m_bits+3) );
    }

    inBits += m_bidirectional ? 3 : 2;
    int ouBits = m_useRCO ? m_bits+3 : m_bits;
    int height = (ouBits > inBits) ? ouBits : inBits;

    m_area.setHeight( (height+1)*8 );
}

void BinCounter::setBits( int b )
{
    if( b < 1 ) b = 1;
    m_bits = b;
    m_topValue = pow( 2, b )-1;
    updatePins();
}

void BinCounter::setParallelIn( bool p )
{
    m_parallelIn = p;
    if( !p ) m_ldPin->removeConnector();
    m_ldPin->setVisible( p );
    updatePins();

    if( Simulator::self()->isRunning() ) return; // No changes

    if( p ) voltChanged();
    else{
        for( IoPin* pin : m_inpPin )
            pin->changeCallBack( this, false );
    }
}

void BinCounter::setUseRCO( bool rco )
{
    m_useRCO = rco;
    if( !rco )
    {
        m_rcoPin->removeConnector();
        m_rboPin->removeConnector();
    }
    m_rcoPin->setVisible( rco );
    m_rboPin->setVisible( rco );
    updatePins();
}

void BinCounter::setBidirectional( bool b )
{
    m_bidirectional = b;
    if( !b ) m_dirPin->removeConnector();
    m_dirPin->setVisible( b );
    updatePins();
}

/*void BinCounter::setSrInv( bool inv )
{
    m_resetInv = inv;
    m_rstPin->setInverted( inv );
}*/

