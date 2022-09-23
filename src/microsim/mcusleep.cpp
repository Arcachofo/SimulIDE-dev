/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "mcusleep.h"
#include "e_mcu.h"

McuSleep::McuSleep( eMcu* mcu, QString name )
       : McuModule( mcu, name )
       , eElement( mcu->getId()+"-"+name )
{
}
McuSleep::~McuSleep(){}


/*void McuSleep::sleep()
{
    qDebug() << "McuSleep Enter Sleep\n";
}*/

void McuSleep::callBack()
{
    qDebug() << "McuSleep Exit Sleep\n";
    m_mcu->sleep( false );
}
