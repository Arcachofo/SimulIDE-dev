/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "zener.h"
#include "itemlibrary.h"

#define tr(str) simulideTr("Zener",str)

Component* Zener::construct( QString type, QString id )
{ return new Zener( type, id ); }

LibraryItem* Zener::libraryItem()
{
    return new LibraryItem(
        tr("Zener Diode"),
        "Rectifiers",
        "zener.png",
        "Zener",
        Zener::construct);
}

Zener::Zener( QString type, QString id )
     : Diode( type, id, true )
{
    m_enumUids = m_enumNames = m_zeners.keys();
}
Zener::~Zener(){}
