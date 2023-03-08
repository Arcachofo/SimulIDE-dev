/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "mcuconfigword.h"

ConfigWord::ConfigWord( eMcu* mcu, QString name )
{
    m_mcu  = mcu;
    m_name = name;
}
ConfigWord::~ConfigWord(){}

bool ConfigWord::setCfgWord( uint16_t addr, uint16_t data )
{
    if( m_cfgWords.contains( addr ) )
    {
        m_cfgWords[addr] = data;
        qDebug() <<"    Loaded Config Word at:"<<"0x"+QString::number( addr, 16 ).toUpper()<<" value = 0x"+QString::number( data, 16 ).toUpper();
        return true;
    }
    return false;
}

uint16_t ConfigWord::getCfgWord( uint16_t addr )
{
    if( addr ) return m_cfgWords.value( addr );
    return m_cfgWords.values().first();
}

