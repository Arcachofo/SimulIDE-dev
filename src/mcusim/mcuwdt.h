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

#ifndef MCUWDT_H
#define MCUWDT_H

#include "mcuprescaled.h"
#include "e-element.h"

class eMcu;

class MAINMODULE_EXPORT McuWdt : public McuPrescaled, public eElement
{
        friend class McuCreator;

    public:
        McuWdt( eMcu* mcu, QString name );
        ~McuWdt();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void reset(){;}

        bool enabled() { return m_enabled; }
        void enable( bool en ) { m_enabled = en; }

    protected:

        bool m_enabled;
        bool m_ovfInter;
        bool m_ovfReset;

        uint64_t m_ovfPeriod; // overflow period in ps
        uint64_t m_clkPeriod; // clock period in ps
};
#endif
