/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef PICPORT_H
#define PICPORT_H

#include "mcuport.h"

class Mcu;
class eMcu;
class McuPin;
class Component;

class MAINMODULE_EXPORT PicPort : public McuPort
{
    public:
        PicPort( eMcu* mcu, QString name );
        ~PicPort();

        virtual void configureA( uint8_t newANSEL ) override;

    protected:
        virtual McuPin* createPin( int i, QString id , Component* mcu ) override;
};

#endif
