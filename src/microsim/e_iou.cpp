/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "e_iou.h"
#include "cpubase.h"
#include "ioport.h"
#include "watcher.h"

eIou::eIou( Mcu* comp, QString id )
    : eElement( id )
{
    m_component = comp;

    m_cpu     = nullptr;
    m_clkPin  = nullptr;
    m_watcher = nullptr;
}
eIou::~eIou()
{}

IoPort* eIou::getIoPort( QString name )
{
    IoPort* port = m_ioPorts.value( name );
    return port;
}

IoPin* eIou::getIoPin( QString pinName )
{
    if( pinName.isEmpty() ) return nullptr;
    IoPin* pin = nullptr;

    for( IoPort* port : m_ioPorts )
    {
        pin = port->getPin( pinName );
        if( pin ) break;
    }
    return pin;
}

void eIou::createWatcher( CpuBase* cpu )
{
    if( !m_watcher ) m_watcher = new Watcher( nullptr, cpu );
}
