/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICSLEEP_H
#define PICSLEEP_H

#include "mcusleep.h"

class PicSleep : public McuSleep
{
    public:
        PicSleep( eMcu* mcu, QString name );
        ~PicSleep();
};
#endif
