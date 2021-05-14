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

class eSource;

class MAINMODULE_EXPORT SpiModule : public eClockedDevice
{
    public:
        SpiModule( QString name );
        ~SpiModule();

        void setMosiPin( eSource* pin );
        void setMisoPin( eSource* pin );
        void setSckPin( eSource* pin );
        void setSsPin( eSource* pin );

    protected:

        eSource* m_MOSI;
        eSource* m_MISO;
        eSource* m_SCK;
        eSource* m_SS;
};
#endif

