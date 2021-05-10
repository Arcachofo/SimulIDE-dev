/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
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

#ifndef EI2CMASTER_H
#define EI2CMASTER_H

#include "e-i2c.h"
#include "component.h"

class MAINMODULE_EXPORT eI2CMaster : public eI2C
{
    public:

        eI2CMaster( QString id );
        ~eI2CMaster();

        virtual void stamp() override;
        virtual void runEvent() override;

        virtual void masterStart( uint8_t addr );
        virtual void masterWrite( uint8_t data );
        virtual void masterRead( uint8_t ack );

    protected:
        virtual void setSDA( bool state ) override;
        virtual void setSCL( bool state ) override;
        void keepClocking();

        void updatePins();

        bool m_toggleScl;

        uint8_t m_masterACK;
};

#endif
