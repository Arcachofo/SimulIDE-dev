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

    m_ciPin = createPin("IR01Ci", m_id+"-ci");
    m_coPin = createPin("OR03Co", m_id+"-co");
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
    for( int i=0; i<m_bits; ++i )
    {
        m_inA[i]->changeCallBack( this );
        m_inB[i]->changeCallBack( this );
    }
    m_ciPin->changeCallBack( this );

    IoComponent::initState();
}

void FullAdder::voltChanged()
{
    int Ci = m_ciPin->getInpState() ? 1 : 0;

    int A=0, B=0;
    for( int i=0; i<m_bits; ++i )
    {
        if( m_inA[i]->getInpState() ) A |= 1<<i;
        if( m_inB[i]->getInpState() ) B |= 1<<i;
    }
    m_nextOutVal = A + B + Ci;

    scheduleOutPuts( this );
}

void FullAdder::setBits( int b )
{
    if( b < 1 ) b = 1;
    if( b == m_bits ) return;

    m_outPin.resize( b+1 );

    if( b > m_bits )  // Add pins
    {
        m_inA.resize( b );
        m_inB.resize( b );
        m_outS.resize( b );

        for( int i=0; i<b; ++i )
        {
            int yB = i+b+2;

            if( i < m_bits ) m_inB.at(i)->setY( yB*8 );
            else{
                QString iStr = QString::number(i);

                QString posStr = QString::number( i+1 );
                if( posStr.size() < 2 ) posStr.prepend("0");
                m_inA.at(i) = createPin("IL"+posStr+"A"+iStr, m_id+"-inA"+iStr );

                posStr = QString::number( yB );
                if( posStr.size() < 2 ) posStr.prepend("0");
                m_inB.at(i) = createPin("IL"+posStr+"B"+iStr, m_id+"-inB"+iStr );

                posStr = QString::number( i+2 );
                if( posStr.size() < 2 ) posStr.prepend("0");
                m_outS.at(i) = createPin("OR"+posStr+"S"+iStr, m_id+"-outS"+iStr );
                m_outPin.at(i) = m_outS.at(i);
            }
        }
    }
    else if( b < m_bits ) // Remove pins
    {
        int toDelete = m_bits-b;
        deletePins( &m_inA, toDelete );
        deletePins( &m_inB, toDelete );
        deletePins( &m_outS, toDelete );

        for( int i=0; i<b; ++i ) m_inB.at(i)->setY( (i+b+2)*8 );
    }

    QString iStr = ( b > 1 ) ? "0" : "";
    m_inA.at(0)->setLabelText("A"+iStr);
    m_inB.at(0)->setLabelText("B"+iStr);
    m_outS.at(0)->setLabelText("S"+iStr);

    m_outPin.at( b ) = m_coPin;
    m_coPin->setY( b*8 + 16 );
    m_bits = b;

    m_height = m_bits*2+2;
    if( m_bits == 1 ) m_height = 4;
    m_area = QRect(m_area.x(), 0, m_width*8, m_height*8 );

    Circuit::self()->update();
}

void FullAdder::remove()
{
    for( int i=0; i<m_bits; ++i )
    {
        m_inA[i]->removeConnector();
        m_inB[i]->removeConnector();
    }
    m_ciPin->removeConnector();
    m_coPin->removeConnector();

    IoComponent::remove();
}
