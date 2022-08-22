/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef MCUPRESCALED_H
#define MCUPRESCALED_H

#include "mcumodule.h"

class MAINMODULE_EXPORT McuPrescaled : public McuModule
{
        friend class McuCreator;

    public:
        McuPrescaled( eMcu* mcu, QString name );
        virtual ~McuPrescaled();

    protected:
        regBits_t m_prSelBits;              // Bits configuring prescaler index
        uint8_t  m_prIndex;                 // Prescaler index
        uint16_t m_prescaler;               // Actual Prescaler value
        std::vector<uint16_t> m_prescList;  // Prescaler values
};

#endif
