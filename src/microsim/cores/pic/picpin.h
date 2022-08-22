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

#ifndef PicPIN_H
#define PicPIN_H

#include "mcupin.h"

class Component;
class McuPort;
class eMcu;

class MAINMODULE_EXPORT PicPin : public McuPin
{
    friend class McuPort;

    public:
        PicPin( McuPort* port, int i, QString id , Component* mcu );
        ~PicPin();

        virtual void ConfExtInt( uint8_t bits ) override;

        virtual void setAnalog( bool an ) override;

    protected:

};

#endif
