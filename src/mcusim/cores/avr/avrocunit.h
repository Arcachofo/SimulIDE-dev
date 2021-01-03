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

#ifndef AVROCUNIT_H
#define AVROCUNIT_H

#include "mcuocunit.h"
#include "avrtimer.h"

class eMcu;
class McuPin;
class McuTimer;

class MAINMODULE_EXPORT AvrOcUnit : public McuOcUnit
{
        friend class McuCreator;

    public:
        AvrOcUnit( eMcu* mcu, QString name );
        ~AvrOcUnit();

        //virtual void initialize() override;
        virtual void configure( uint8_t val ) override;

    protected:
        //void reConfigure();
};

#endif
