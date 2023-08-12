/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picsleep.h"

PicSleep::PicSleep( eMcu* mcu, QString name )
        : McuSleep( mcu, name )
{
    m_enabled = true;
    m_sleepMode = 1;
}
PicSleep::~PicSleep(){}
