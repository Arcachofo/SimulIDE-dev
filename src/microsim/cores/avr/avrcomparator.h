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

#ifndef AVRCOMPARATOR_H
#define AVRCOMPARATOR_H

#include "mcucomparator.h"
#include "mcutypes.h"

class MAINMODULE_EXPORT AvrComp : public McuComp
{
    public:
        AvrComp( eMcu* mcu, QString name );
        ~AvrComp();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configureA( uint8_t newACSR ) override;
        virtual void configureB( uint8_t newAIND ) override;
        virtual void configureC( uint8_t newACOE ) override;

    protected:
        void compare( uint8_t );

        regBits_t m_ACD;
        regBits_t m_ACBG;
        regBits_t m_ACO;
        regBits_t m_ACI;
        regBits_t m_ACIC;
        regBits_t m_ACIS;

        regBits_t m_AIN0D;
        regBits_t m_AIN1D;
};

#endif
