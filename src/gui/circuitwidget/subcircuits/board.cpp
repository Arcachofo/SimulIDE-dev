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
    //m_shield = NULL;
}
BoardSubc::~BoardSubc(){}

void BoardSubc::attachShield( ShieldSubc* shield )
{
    if( !m_shields.contains( shield ) ) m_shields.append( shield );
}

void BoardSubc::remove()
{
    for( ShieldSubc* shield : m_shields ) // there is a shield attached to this
    {
        shield->setBoard( NULL );
        Circuit::self()->removeComp( shield );
    }
    SubCircuit::remove();
}
