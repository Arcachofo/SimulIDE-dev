/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "magnitudecomp.h"
#include "itemlibrary.h"
#include "iocomponent.h"
#include "iopin.h"

#include "intprop.h"

#define tr(str) simulideTr("MagnitudeComp",str)

Component* MagnitudeComp::construct( QString type, QString id )
{ return new MagnitudeComp( type, id ); }

LibraryItem* MagnitudeComp::libraryItem()
{
    return new LibraryItem(
        tr("Magnitude Comparator"),
        "Arithmetic" ,
        "3to2g.png",
        "MagnitudeComp",
        MagnitudeComp::construct );
}

MagnitudeComp::MagnitudeComp( QString type, QString id )
             : IoComponent( type, id )
             , eElement( id )
{
    m_width  = 4;
    m_height = 4;

    init({          // Inputs:
            "IL01iA>B",
            "IL02iA=B",
            "IL03iA<B",
                   // Outputs:
            "OR01A>B",
            "OR02A=B",
            "OR03A<B"
        });

    setBits( 2 );

    addPropGroup( { tr("Main"), {
        new IntProp <MagnitudeComp>("Bits", tr("Size"),"_bits"
                            , this, &MagnitudeComp::bits, &MagnitudeComp::setBits, propNoCopy,"uint" ),
    }, groupNoCopy } );

    addPropGroup( { tr("Electric")
                    , IoComponent::inputProps()
                    + IoComponent::outputProps()
                    + IoComponent::outputType(),0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}

MagnitudeComp::~MagnitudeComp(){}

void MagnitudeComp::stamp()
{
    IoComponent::initState();
    m_outPin[0]->setOutState( true );
    m_outPin[2]->setOutState( true );

    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
}

void MagnitudeComp::voltChanged() // Called when any pin node change volt
{
    int iA = 0, A = 0, B = 0;

    for( int i=0; i<3; ++i )
        if( m_inPin[i]->getInpState() ) iA |= 1<<i;

    for( int i=0; i<m_bits; ++i )
    {
        if( m_inPin[3+i]->getInpState() ) A |= 1<<i;
        if( m_inPin[3+m_bits+i]->getInpState() ) B |= 1<<i;
    }

    if     ( A > B ) m_nextOutVal = 0b001;
    else if( A < B ) m_nextOutVal = 0b100;
    else{
        /*if     ( iA  & 0b010 ) iA = 0b010; // Logic for 7485
        else if( iA == 0b101 ) iA = 0b000;
        else if( iA == 0b000 ) iA = 0b101;*/
        m_nextOutVal = iA;
    }
    scheduleOutPuts( this );
}

void MagnitudeComp::setBits( int b )
{
    if( b < 1 ) b = 1;
    m_bits = b;

    setNumInps( 3+b*2, "I"); // Set a label to get Pin 1 full cell below edge

    for( int i=0; i<b; ++i ){
        m_inPin[3+i]->setLabelText("A"+QString::number(i) );
        m_inPin[3+b+i]->setLabelText("B"+QString::number(i) );
    }
}
