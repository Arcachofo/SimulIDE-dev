/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "ampmeter.h"
#include "itemlibrary.h"

Component* Amperimeter::construct( QObject* parent, QString type, QString id )
{ return new Amperimeter( parent, type, id ); }

LibraryItem* Amperimeter::libraryItem()
{
    return new LibraryItem(
        QObject::tr( "Ampmeter" ),
        QObject::tr( "Meters" ),
        "amperimeter.png",
        "Amperimeter",
        Amperimeter::construct);
}

Amperimeter::Amperimeter( QObject* parent, QString type, QString id )
           : Meter( parent, type, id )
{
    m_unit = "A";
    m_dispValue = 0;
    setRes( 1e-6 );
    m_display.setText( "0.000\nA");
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

#include "moc_ampmeter.cpp"
