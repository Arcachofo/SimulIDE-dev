/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "fulladder.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "iopin.h"

#include "intprop.h"

#define tr(str) simulideTr("FullAdder",str)

Component* FullAdder::construct( QString type, QString id)
{ return new FullAdder( type, id); }

LibraryItem* FullAdder::libraryItem()
{
    return new LibraryItem(
        tr("Full Adder"),
        "Arithmetic",
        "2to2.png",
        "FullAdder",
        FullAdder::construct );
}

FullAdder::FullAdder( QString type, QString id)
         : IoComponent( type, id )
         , eElement( id )
{
    m_width  = 3;
    m_height = 4;
    m_bits   = 0;

    m_area = QRect( -8, -(m_height/2)*8, m_width*8, m_height*8 );
    m_inPin.resize( 1 );
    m_outPin.resize( 1 );

    m_inPin[0]  = m_ciPin = createPin("IR01Ci", m_id+"-ci");
    m_outPin[0] = m_coPin = createPin("OR03Co", m_id+"-co");
    setBits( 1 );

    addPropGroup( { tr("Main"), {
        new IntProp <FullAdder>("Bits", tr("Size"),"_bits"
                            , this, &FullAdder::bits, &FullAdder::setBits, propNoCopy,"uint" ),
    }, groupNoCopy} );

    addPropGroup( { tr("Electric"), IoComponent::inputProps()
                                  + IoComponent::outputProps(),0 } );

    addPropGroup( { tr("Timing")  , IoComponent::edgeProps(),0 } );
}
FullAdder::~FullAdder(){}

void FullAdder::stamp()
{
    IoComponent::initState();
    for( IoPin* pin : m_inPin ) pin->changeCallBack( this );
}

void FullAdder::voltChanged()
{
    int Ci = m_ciPin->getInpState() ? 1 : 0;

    int A=0, B=0;
    for( int i=0; i<m_bits; ++i )
    {
        if( m_inPin[1+i]->getInpState()        ) A |= 1<<i;
        if( m_inPin[1+i+m_bits]->getInpState() ) B |= 1<<i;
    }
    m_nextOutVal = A + B + Ci;

    bool Co = m_nextOutVal & 1<<m_bits; // Move Carry out to first bit: m_outPin[0] = m_coPin
    m_nextOutVal <<= 1;
    if( Co ) m_nextOutVal |= 1;

    scheduleOutPuts( this );
}

void FullAdder::setBits( int b )
{
    if( b < 1 ) b = 1;
    if( b == m_bits ) return;

    setNumInps( 1+b*2, "I", 0, 10 ); // Set a label to get Pin 1 full cell below edge
    setNumOuts( 1+b  , "S", 0, 10 );

    for( int i=0; i<b; ++i )
    {
        QString iStr = ( b > 1 ) ? QString::number(i) : "";

        m_inPin[1+i]->setY( m_area.y()+i*8+8 );
        m_inPin[1+i]->setLabelText("A"+iStr );
        m_inPin[1+b+i]->setLabelText("B"+iStr );
        m_outPin[1+i]->setLabelText("S"+iStr );
    }

    m_coPin->setY( m_area.y()+b*8+16 );

    m_bits = b;

    Circuit::self()->update();
}
