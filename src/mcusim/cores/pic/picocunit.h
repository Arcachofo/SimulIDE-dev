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

#ifndef PICOCUNIT_H
#define PICOCUNIT_H

#include "mcuocunit.h"
//#include "pictimer.h"


class MAINMODULE_EXPORT PicOcUnit : public McuOcUnit
{
        friend class McuCreator;

    public:
        PicOcUnit( eMcu* mcu, QString name );
        ~PicOcUnit();

        //virtual void initialize() override;
        virtual void runEvent();

        virtual void configure( uint8_t val ) override;

    protected:
        bool m_enhanced;
        bool m_specEvent;
        bool m_resetTimer;

        regBits_t m_GODO;
};

#endif
