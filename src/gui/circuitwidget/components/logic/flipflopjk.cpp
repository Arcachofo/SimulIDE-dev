/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "flipflopjk.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* FlipFlopJK::construct( QObject* parent, QString type, QString id )
{ return new FlipFlopJK( parent, type, id ); }

LibraryItem* FlipFlopJK::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("FlipFlopJK", "FlipFlop JK"),
        "Memory",
        "3to2.png",
        "FlipFlopJK",
        FlipFlopJK::construct );
}

FlipFlopJK::FlipFlopJK( QObject* parent, QString type, QString id )
          : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 4;
    m_dataPins = 2;

    init({          // Inputs:
            "IL01J",
            "IL03K",
            "IU01S",
            "ID02R",
            "IL02>",
                   // Outputs:
            "OR01Q",
            "OR03!Q",
        });

    m_setPin   = m_inPin[2];
    m_resetPin = m_inPin[3];
    m_clkPin   = m_inPin[4];

    setSrInv( true );           // Invert Set & Reset pins
    setClockInv( false );       //Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopJK::~FlipFlopJK(){}

void FlipFlopJK::calcOutput()
{
    bool J = m_inPin[0]->getInpState();
    bool K = m_inPin[1]->getInpState();
    bool Q = m_outPin[0]->getOutState();

    m_Q0 = (J && !Q) || (!K && Q) ;
    m_nextOutVal = m_Q0? 1:2;
}
