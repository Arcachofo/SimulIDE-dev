/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef MCUPORT_H
#define MCUPORT_H

#include<QHash>

#include "mcumodule.h"
#include "e-element.h"

class Mcu;
class eMcu;
class McuPin;
class Component;

class MAINMODULE_EXPORT McuPort : public McuModule, public eElement
{
        friend class McuCreator;
        friend class McuPorts;

    public:
        McuPort( eMcu* mcu, QString name, uint8_t numPins );
        ~McuPort();

        virtual void initialize() override;

        virtual void pinChanged( uint8_t pinMask, uint8_t val );

        void setPullups( uint8_t puMask );

        McuPin* getPin( uint8_t i ) { return m_pins[i]; }

        QString name() { return m_name; }

        void outChanged( uint8_t val );
        void dirChanged( uint8_t val );
        void intMaskChanged( uint8_t val ) { m_intMask = val; }

        uint16_t getOutAddr() { return m_outAddr; }
        uint16_t getInAddr() { return m_inAddr; }

    protected:
        virtual void createPins( Mcu* mcuComp );

        std::vector<McuPin*> m_pins;
        uint8_t m_numPins;
        uint8_t m_pinState;

        uint8_t m_intMask;

        bool m_dirInv;

        uint8_t* m_outReg; // Pointer to m_ram[m_outAddr]
        uint8_t* m_dirReg; // Pointer to m_ram[m_dirAddr]

        uint16_t m_outAddr;
        uint16_t m_inAddr;
        uint16_t m_dirAddr;
};

class MAINMODULE_EXPORT McuPorts
{
        friend class McuCreator;

    public:
        McuPorts( eMcu* mcu );
        ~McuPorts();

       McuPort* getPort( QString name ) { return m_portList.value( name ); }
       McuPin*  getPin( QString name );
       QHash<QString, McuPort*> getPorts() { return m_portList; }

    protected:
       eMcu* m_mcu;

       QHash<QString, McuPort*> m_portList; // Access PORTS by name
};

#endif
