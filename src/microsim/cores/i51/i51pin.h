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

#ifndef I51PIN_H
#define I51PIN_H

#include "mcupin.h"

class Component;
class McuPort;
class eMcu;

class MAINMODULE_EXPORT I51Pin : public McuPin
{
    friend class McuPort;

    public:
        I51Pin( McuPort* port, int i, QString id , Component* mcu );
        ~I51Pin();

        //virtual void initialize() override;

        virtual void setOutState( bool state ) override;

        virtual void ConfExtInt( uint8_t bits ) override;

    protected:

};

#endif
