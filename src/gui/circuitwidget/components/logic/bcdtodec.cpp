/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "bcdtodec.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "iopin.h"

#include "boolprop.h"

#define tr(str) simulideTr("BcdToDec",str)

Component* BcdToDec::construct( QString type, QString id )
{ return new BcdToDec( type, id ); }

LibraryItem* BcdToDec::libraryItem()
{
    return new LibraryItem(
        tr("Decoder(4 to 10/16)"),
        "Converters",
        "2to3g.png",
        "BcdToDec",
        BcdToDec::construct );
}

BcdToDec::BcdToDec( QString type, QString id )
        : LogicComponent( type, id )
{
    m_width  = 4;
    m_height = 11;

    m_tristate = true;
    m_16Bits   = false;

    setNumInps( 4,"S" );
    setNumOuts( 10 );
    createOePin( "IU01OE ", id+"-in4"); // Output Enable

    addPropGroup( { tr("Main"), {
        new BoolProp<BcdToDec>("_16_Bits", tr("16 Bits"),""
                              , this, &BcdToDec::is16Bits, &BcdToDec::set_16bits, propNoCopy )
    },groupNoCopy} );

    addPropGroup( { tr("Electric"), IoComponent::inputProps()
        +QList<ComProperty*>({
        new BoolProp<BcdToDec>("Invert_Inputs", tr("Invert Inputs"),""
                              , this, &BcdToDec::invertInps, &BcdToDec::setInvertInps, propNoCopy )})

        +IoComponent::outputProps()+IoComponent::outputType(),0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
BcdToDec::~BcdToDec(){}

void BcdToDec::stamp()
{
    for( int i=0; i<4; ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp();

    m_outPin[0]->setOutState( true );
    m_outValue = 1;
}

void BcdToDec::voltChanged()
{
    LogicComponent::updateOutEnabled();

    int dec = 0;
    for( int i=0; i<4; ++i )
        if( m_inPin[i]->getInpState() ) dec += pow( 2, i );

    m_nextOutVal = 1<<dec;
    scheduleOutPuts( this );
}

void BcdToDec::set_16bits( bool set )
{
    m_16Bits = set;
    setNumOuts( m_16Bits ? 16 : 10 );
    m_oePin->setY( m_area.y()-8 );
}
