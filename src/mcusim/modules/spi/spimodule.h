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

#ifndef SPIMODULE_H
#define SPIMODULE_H

#include "e-clocked_device.h"

class IoPin;

class MAINMODULE_EXPORT SpiModule : public eClockedDevice
{
    public:
        SpiModule( );
        ~SpiModule();

        void setMosiPin( IoPin* pin );
        void setMisoPin( IoPin* pin );
        void setSckPin( IoPin* pin );
        void setSsPin( IoPin* pin );

    protected:

        IoPin* m_MOSI;
        IoPin* m_MISO;
        IoPin* m_SCK;
        IoPin* m_SS;
};
#endif

