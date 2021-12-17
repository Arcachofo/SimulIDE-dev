/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
    uint8_t bit0;
    uint8_t mask;
    uint8_t* reg;
    uint16_t regAddr;
};

#endif
