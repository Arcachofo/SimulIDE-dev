/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "ampmeter.h"
#include "itemlibrary.h"

#define tr(str) simulideTr("Amperimeter",str)

Component* Amperimeter::construct( QString type, QString id )
{ return new Amperimeter( type, id ); }

LibraryItem* Amperimeter::libraryItem()
{
    return new LibraryItem(
        tr("Ampmeter"),
        "Meters",
        "amperimeter.png",
        "Amperimeter",
        Amperimeter::construct);
}

Amperimeter::Amperimeter( QString type, QString id )
           : Meter( type, id )
{
    m_unit = "A";
    m_dispValue = 0;
    setResistance( 1e-6 );
    m_display.setText(" 0.000\n A");
}
Amperimeter::~Amperimeter(){}

void Amperimeter::updateStep()
{
    double curr = current();
    
    if( curr != m_dispValue )
    {
        m_dispValue = curr;
        Meter::updateStep();
}   }
