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

#ifndef PICICUNIT_H
#define PICICUNIT_H

#include "mcuicunit.h"


class MAINMODULE_EXPORT PicIcUnit : public McuIcUnit
{
    friend class PicCcpUnit;

    public:
        PicIcUnit( eMcu* mcu, QString name );
        virtual ~PicIcUnit();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configure( uint8_t CCPxM ) override;

    private:
        uint64_t m_prescaler;
        uint64_t m_counter;
};

#endif
