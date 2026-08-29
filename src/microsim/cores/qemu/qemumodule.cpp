/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "qemumodule.h"
#include "utils.h"

QemuModule::QemuModule( QemuDevice* mcu, QString name, int n, uint32_t* clk, uint64_t memStart, uint64_t memEnd )
{
    m_device = mcu;
    m_number = n;
    m_name   = name;
    m_frequency = clk;

    m_memStart = memStart;
    m_memEnd   = memEnd;

    m_ioMem = m_device->getIoMem();
    m_arena = m_device->getArena();

    m_device->addModule( this );
    //qDebug() << "QemuModule::QemuModule" << m_name << toHex32( memStart ) << toHex32( memEnd ) << clk;
}
QemuModule::~QemuModule(){}

void QemuModule::reset()
{
    //nextEvent = nullptr;
    //eventTime = 0;
}

void QemuModule::runAction()
{
    if     ( m_eventAction == SIM_READ  ) readRegister();
    else if( m_eventAction == SIM_WRITE ) writeRegister();
    else qDebug() << "QemuModule::runAction ERROR: wrong action";
}

void QemuModule::writeRegister()
{
    (*m_ioMem)[m_eventAddress] = m_eventValue;
}

void QemuModule::readRegister()
{
    uint32_t data = (*m_ioMem)[m_eventAddress];
    //qDebug() << "QemuModule::runEvent Read" << m_eventAddress << data;
    m_arena->regData = data;
    m_arena->qemuAction = SIM_READ;
}

void QemuModule::setInterrupt( uint8_t number, uint8_t level )
{
    m_arena->irqNumber = number;
    m_arena->irqLevel  = level? 1:0;
    //m_arena->qemuAction = SIM_INTERRUPT;
    //qDebug() << "QemuModule::setInterrupt" << m_name << number << level;
}
