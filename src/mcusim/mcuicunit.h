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

#ifndef MCUICUNIT_H
#define MCUICUNIT_H

#include "mcumodule.h"
#include "e-element.h"

class McuPin;
class McuTimer;

class MAINMODULE_EXPORT McuIcUnit : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuIcUnit( eMcu* mcu, QString name );
        virtual ~McuIcUnit();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configure( uint8_t val ) {;}

    protected:
        McuTimer* m_timer;
        McuPin*   m_icPin;

        uint8_t* m_icRegL;
        uint8_t* m_icRegH;

        bool m_enabled;
        bool m_inState;

        uint8_t m_mode;

        bool m_fallingEdge;
};

#endif
