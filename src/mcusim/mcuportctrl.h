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

#ifndef MCUCTRLPORT_H
#define MCUCTRLPORT_H

#include "mcumodule.h"

class Mcu;
class IoPin;

class MAINMODULE_EXPORT McuCtrlPort : public McuModule
{
        friend class McuCreator;

    public:
        McuCtrlPort( eMcu* mcu, QString name);
        ~McuCtrlPort();

        //IoPin* getPinN( uint8_t i );
        IoPin* getPin( QString pinId );

    protected:
         virtual void createPins( Mcu* mcuComp, QString pins );

        std::vector<IoPin*> m_pins;
        //uint8_t m_numPins;
};

#endif
