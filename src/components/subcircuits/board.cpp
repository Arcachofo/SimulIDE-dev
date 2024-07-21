/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "board.h"
#include "circuit.h"
#include "shield.h"

BoardSubc::BoardSubc( QString type, QString id )
         : SubCircuit( type, id )
{
    m_graphical = true;
    m_subcType = Chip::Board;
    m_parentBoard = NULL;
}
BoardSubc::~BoardSubc(){}

void BoardSubc::setLogicSymbol( bool ls ) /// FIXME
{
    if( m_shields.size() ) return;
    SubCircuit::setLogicSymbol( ls );
}

void BoardSubc::attachShield( ShieldSubc* shield )
{
    if( !m_shields.contains( shield ) ) m_shields.append( shield );
}

QString BoardSubc::toString()
{
    QString toStr;
    for( ShieldSubc* shield : m_shields )
    {
        toStr += shield->toString();
    }
    toStr += SubCircuit::toString();
    return toStr;
}

