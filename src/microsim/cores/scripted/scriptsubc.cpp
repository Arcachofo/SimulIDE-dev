/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scriptsubc.h"
#include "itemlibrary.h"
#include "mcucreator.h"
#include "circuit.h"

LibraryItem* ScriptSubc::libraryItem()
{
    return new LibraryItem(
        "Script_SubCircuit",
        "",
        "ic2.png",
        "ScriptSubc",
        ScriptSubc::construct );
}

Component* ScriptSubc::construct( QString type, QString id )
{
    m_error = 0;
    ScriptSubc* mcu = new ScriptSubc( type, id );
    if( !m_error) m_error = McuCreator::createMcu( mcu, id );

    if( m_error > 0 )
    {
        Circuit::self()->removeComp( mcu );
        mcu = nullptr;
        m_pSelf = nullptr;
        m_error = 0;
    }
    return mcu;
}

ScriptSubc::ScriptSubc( QString type, QString id )
          : Mcu( type, id )
          , EmbedCircuit( m_name, id, this )
{

}
ScriptSubc::~ScriptSubc(){}
