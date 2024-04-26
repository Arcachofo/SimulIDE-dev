/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "voltmeter.h"
#include "itemlibrary.h"
#include "e-pin.h"

#define tr(str) simulideTr("Voltimeter",str)

Component* Voltimeter::construct( QString type, QString id )
{ return new Voltimeter( type, id ); }

LibraryItem* Voltimeter::libraryItem()
{
    return new LibraryItem(
        tr("Voltmeter"),
        "Meters",
        "voltimeter.png",
        "Voltimeter",
        Voltimeter::construct);
}

Voltimeter::Voltimeter( QString type, QString id )
          : Meter( type, id )
{
    m_unit = "V";
    setResistance( high_imp );
    m_display.setText( " 0.000\n V");
}
Voltimeter::~Voltimeter(){}

void Voltimeter::updateStep()
{
    double volt = m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage();
    
    if( volt != m_dispValue )
    {
        m_dispValue = volt;
        Meter::updateStep();
}   }
