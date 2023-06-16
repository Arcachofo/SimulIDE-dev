/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "fulladder.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* FullAdder::construct(QObject *parent, QString type, QString id)
{ return new FullAdder(parent, type, id); }

LibraryItem* FullAdder::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("FullAdder", "Full Adder"),
        "Arithmetic",
        "2to2.png",
        "FullAdder",
        FullAdder::construct );
}

FullAdder::FullAdder(QObject *parent, QString type, QString id) 
         : LogicComponent( parent, type, id )
{
    m_width  = 3;
    m_height = 4;

    init({         // Inputs:
            "IL01A",
            "IL03B",
            "IR01Ci",
                    // Outputs:
            "OR02S",
            "OR03Co",
        });

    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps(),0 } );
    addPropGroup( { tr("Edges")   , IoComponent::edgeProps(),0 } );
}
FullAdder::~FullAdder(){}

void FullAdder::stamp()
{
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp();
}

void FullAdder::voltChanged()
{
    bool X  = m_inPin[0]->getInpState();
    bool Y  = m_inPin[1]->getInpState();
    bool Ci = m_inPin[2]->getInpState();

    bool sum = (X ^ Y) ^ Ci;                    // Sum
    bool co  = (X & Ci) | (Y & Ci) | (X & Y);   // Carry out

    m_nextOutVal = 0;
    if( sum ) m_nextOutVal += 1;
    if( co  ) m_nextOutVal += 2;
    sheduleOutPuts( this );
}
