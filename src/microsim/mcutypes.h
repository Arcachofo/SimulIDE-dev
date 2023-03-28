/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUTYPES_H
#define MCUTYPES_H

#include <inttypes.h>
#include "mcusignal.h"

struct regInfo_t{
    uint16_t address;
    uint8_t  resetVal;
};

struct regSignal_t{
    McuSignal on_write;
    McuSignal on_read;
};

struct regBits_t{
    uint8_t bit0=0;
    uint8_t mask=0;
    uint8_t* reg=NULL;
    uint16_t regAddr=0;
};

#endif
