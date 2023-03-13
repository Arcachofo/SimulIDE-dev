/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "flipflopd.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* FlipFlopD::construct( QObject* parent, QString type, QString id )
{ return new FlipFlopD( parent, type, id ); }

LibraryItem* FlipFlopD::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("FlipFlopD", "FlipFlop D"),
        "Memory",
        "2to2.png",
        "FlipFlopD",
        FlipFlopD::construct );
}

FlipFlopD::FlipFlopD( QObject* parent, QString type, QString id )
         : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 3;
    m_dataPins = 1;

    init({         // Inputs:
            "IL01D",
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
FlipFlopD::~FlipFlopD(){}

void FlipFlopD::calcOutput()
{
    m_nextOutVal = m_inPin[0]->getInpState()? 1:2; // D state
}
