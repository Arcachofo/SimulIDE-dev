/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "voltmeter.h"
#include "itemlibrary.h"
#include "e-pin.h"

Component* Voltimeter::construct( QObject* parent, QString type, QString id )
{ return new Voltimeter( parent, type, id ); }

LibraryItem* Voltimeter::libraryItem()
{
    return new LibraryItem(
        QObject::tr( "Voltmeter" ),
        QObject::tr( "Meters" ),
        "voltimeter.png",
        "Voltimeter",
        Voltimeter::construct);
}

Voltimeter::Voltimeter( QObject* parent, QString type, QString id )
          : Meter( parent, type, id )
{
    m_unit = "V";
    setRes( high_imp );
    m_display.setText( "0.000\nV");
}
Voltimeter::~Voltimeter(){}

void Voltimeter::updateStep()
{
    double volt = m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage();
    
    if( volt != m_dispValue )
    {
        //setUnit("V");
        m_dispValue = volt;
        Meter::updateStep();
}   }

#include "moc_voltmeter.cpp"
