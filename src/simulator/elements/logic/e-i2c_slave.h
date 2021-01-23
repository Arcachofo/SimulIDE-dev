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

#ifndef EI2CSLAVE_H
#define EI2CSLAVE_H

#include "e-i2c.h"
#include "component.h"


class MAINMODULE_EXPORT eI2CSlave : public eI2C
{
    public:
        eI2CSlave( QString id );
        ~eI2CSlave();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void setAddress( int address );
        virtual int byteReceived() { return m_rxReg; }

    protected:
        virtual void setSDA( bool state ) override;
        virtual void setSCL( bool ) override;

        void updatePins();
        
        int m_address;           // Device Address
        int m_addressBits;       // Number of m_address bits

        bool m_nextSDA;
};


#endif
