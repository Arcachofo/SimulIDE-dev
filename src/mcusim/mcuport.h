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
//#include "e-element.h"

class Mcu;
class eMcu;
class McuPin;
class Component;

class MAINMODULE_EXPORT McuPort : public McuModule//, public eElement
{
        friend class McuCreator;
        friend class McuPorts;

    public:
        McuPort( eMcu* mcu, QString name );
        ~McuPort();

        virtual void reset() override;
        virtual void pinChanged( uint8_t pinMask, uint8_t val );

        void setPullups( uint8_t puMask );
        void setAllPullups( uint8_t val );
        void clearAllPullups( uint8_t val );

        McuPin* getPinN( uint8_t i );
        McuPin* getPin( QString pinName );

        QString name() { return m_name; }

        virtual void outChanged( uint8_t val );
        virtual void dirChanged( uint8_t val );
        virtual void setIntMask( uint8_t val) { m_intMask = val; }
        virtual void rstIntMask( bool rst) { m_rstIntMask = rst; }

        virtual void readPort( uint8_t );

        uint16_t getOutAddr() { return m_outAddr; }
        uint16_t getInAddr() { return m_inAddr; }

    protected:
        void createPins( Mcu* mcuComp, QString pins, uint8_t pinMask=255 );
        virtual McuPin* createPin( int i, QString id , Component* mcu );

        std::vector<McuPin*> m_pins;
        uint8_t m_numPins;
        uint8_t m_pinState;

        uint8_t m_intMask;

        bool m_dirInv;
        bool m_rstIntMask;

        uint8_t* m_outReg; // Pointer to m_ram[m_outAddr]
        uint8_t* m_inReg;  // Pointer to m_ram[m_inAddr]
        uint8_t* m_dirReg; // Pointer to m_ram[m_dirAddr]

        uint16_t m_outAddr;
        uint16_t m_inAddr;
        uint16_t m_dirAddr;
};

#endif
