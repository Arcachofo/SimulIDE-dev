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

#ifndef TWISLAVE_H
#define TWISLAVE_H

#include "twimodule.h"

class MAINMODULE_EXPORT TwiSlave : public TwiTR
{
    public:
        TwiSlave( TwiModule* twi, QString name );
        ~TwiSlave();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

    protected:
        virtual void setSDA( bool state ) override;

        int m_addressBits;
        uint8_t m_address;           // Device Address

        bool m_nextSDA;
};

#endif
