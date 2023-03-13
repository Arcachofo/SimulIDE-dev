/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "flipflopt.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* FlipFlopT::construct( QObject* parent, QString type, QString id )
{ return new FlipFlopT( parent, type, id ); }

LibraryItem* FlipFlopT::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("FlipFlopT", "FlipFlop T" ),
        "Memory",
        "2to2.png",
        "FlipFlopT",
        FlipFlopT::construct );
}

FlipFlopT::FlipFlopT( QObject* parent, QString type, QString id )
         : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 3;
    m_dataPins = 1;

    init({         // Inputs:
            "IL01T",
            "IU01S",
            "ID02R",
            "IL02>",
                   // Outputs:
            "OR01Q",
            "OR02!Q"
        });

    m_setPin   = m_inPin[1];
    m_resetPin = m_inPin[2];
    m_clkPin   = m_inPin[3];

    setSrInv( true );       // Inver Set & Reset pins
    setClockInv( false );   // Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopT::~FlipFlopT(){}

void FlipFlopT::calcOutput()
{
    bool T = m_inPin[0]->getInpState();
    if( T ) m_nextOutVal = m_outPin[1]->getOutState()? 1:2; // !Q state = Toggle
}
