/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "flipflopjk.h"
#include "itemlibrary.h"
#include "iopin.h"

#define tr(str) simulideTr("FlipFlopJK",str)

Component* FlipFlopJK::construct( QString type, QString id )
{ return new FlipFlopJK( type, id ); }

LibraryItem* FlipFlopJK::libraryItem()
{
    return new LibraryItem(
        tr("FlipFlop JK"),
        "Memory",
        "3to2.png",
        "FlipFlopJK",
        FlipFlopJK::construct );
}

FlipFlopJK::FlipFlopJK( QString type, QString id )
          : FlipFlopBase( type, id )
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

    m_setPin = m_inPin[2];
    m_rstPin = m_inPin[3];
    m_clkPin = m_inPin[4];

    m_setPin->setInverted( true );
    m_rstPin->setInverted( true );

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
