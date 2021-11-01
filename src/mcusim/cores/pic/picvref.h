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

#ifndef PICVREF_H
#define PICVREF_H

#include "mcuvref.h"

class MAINMODULE_EXPORT PicVref : public McuVref
{
    public:
        PicVref( eMcu* mcu, QString name );
        ~PicVref();

        //virtual void initialize() override;

        virtual void configureA( uint8_t newVRCON ) override;

    protected:
        //virtual void setMode( uint8_t mode ) override;

        bool m_vrr;
        bool m_vroe;

        regBits_t m_VREN;
        regBits_t m_VROE;
        regBits_t m_VRR;
        regBits_t m_VR;
};

#endif
