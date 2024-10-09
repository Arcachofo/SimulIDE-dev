/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scriptsubc.h"
#include "itemlibrary.h"
#include "mcucreator.h"
#include "circuit.h"
#include "utils.h"

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
    ScriptSubc* scSubc = new ScriptSubc( type, id );
    if( !m_error) m_error = McuCreator::createMcu( scSubc, id );
    if( !m_error){
        QString subcFile = changeExt( scSubc->m_dataFile, "sim1" );
        scSubc->loadSubCircuitFile( subcFile );
    }
    if( m_error )
    {
        Circuit::self()->removeComp( scSubc );
        scSubc = nullptr;
        m_pSelf = nullptr;
        m_error = 0;
    }
    return scSubc;
}

ScriptSubc::ScriptSubc( QString type, QString id )
          : Mcu( type, id )
          , EmbedCircuit( m_name, id, this )
{

}
ScriptSubc::~ScriptSubc(){}

Pin* ScriptSubc::addPin( QString id, QString type, QString label, int, int xpos, int ypos, int angle, int length, int space )
{
    if( m_pinTunnels.contains( m_ecId+"-"+id ) )
    {
        if( m_initialized  )
        {
            return EmbedCircuit::updatePin( id, type, label, xpos, ypos, angle, length );
        }else
            return EmbedCircuit::addPin( id, type, label, 0, xpos, ypos, angle, length, space );
    }
    else return Mcu::addPin( id, type, label, 0, xpos, ypos, angle, length, space );
}

