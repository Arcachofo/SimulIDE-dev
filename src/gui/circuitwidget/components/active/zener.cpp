/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "zener.h"
#include "itemlibrary.h"

Component* Zener::construct( QObject* parent, QString type, QString id )
{ return new Zener( parent, type, id ); }

LibraryItem* Zener::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("Zener", "Zener Diode"),
        "Rectifiers",
        "zener.png",
        "Zener",
        Zener::construct);
}

Zener::Zener( QObject* parent, QString type, QString id )
     : Diode( parent, type, id, true )
{
    m_enumUids = m_enumNames = m_zeners.keys();
}
Zener::~Zener(){}
