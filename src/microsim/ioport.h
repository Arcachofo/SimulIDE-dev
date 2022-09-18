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

#ifndef IOPORT_H
#define IOPORT_H

#include <vector>

#include "iopin.h"

class IoPin;
class Component;

class MAINMODULE_EXPORT IoPort
{
        friend class McuCreator;

    public:
        IoPort( QString name );
        ~IoPort();

        void reset();

        void setOutState( uint32_t val );
        void setOutStatFast( uint32_t val );
        uint32_t getInpState();

        void setDirection( uint32_t val );
        void setPinMode( pinMode_t mode );

        void portVoltChanged( eElement* el, bool ch );

        IoPin* getPinN( uint8_t i );
        IoPin* getPin( QString pinName );

        QString name() { return m_name; }

    protected:
        void createPins( Component* comp, QString pins, uint32_t pinMask );
        virtual IoPin* createPin( int i, QString id , Component* comp );

        QString m_name;
        QString m_shortName;

        std::vector<IoPin*> m_pins;
        uint8_t m_numPins;
        uint m_pinState;
        uint m_pinDirection;
};

#endif
