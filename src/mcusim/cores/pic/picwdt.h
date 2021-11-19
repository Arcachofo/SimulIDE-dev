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

#ifndef PICWDT_H
#define PICWDT_H

#include "mcuwdt.h"

class MAINMODULE_EXPORT PicWdt : public McuWdt
{
    public:
        PicWdt( eMcu* mcu, QString name );
        ~PicWdt();

        virtual void initialize() override;
        //virtual void runEvent() override;

        virtual void configureA( uint8_t newOPTION ) override;

        virtual void reset() override;

    private:
        regBits_t m_PS;
        regBits_t m_PSA;

};
#endif
