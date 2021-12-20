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

#ifndef MCUPIN_H
#define MCUPIN_H

#include "iopin.h"
#include "mcutypes.h"

class McuPort;
class eMcu;
class Interrupt;

class MAINMODULE_EXPORT McuPin : public IoPin
{
    friend class McuPort;
    friend class McuCreator;

    public:
        McuPin( McuPort* port, int i, QString id , Component* mcu );
        ~McuPin();

        enum extIntTrig_t{
            pinLow=0,
            pinChange,
            pinFalling,
            pinRising
        };

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        virtual void setOutState( bool state ) override;
        virtual void setPortState( bool state );

        void setDirection( bool out );
        void setPullupMask( bool up ) { m_puMask = up;}
        void setExtraSource( double vddAdmit, double gndAdmit );

        virtual void setAnalog( bool an ) {;}
        virtual void ConfExtInt( uint8_t bits );

    protected:
        QString m_id;

        McuPort*   m_port;
        Interrupt* m_extInt;

        regBits_t    m_extIntBits;
        extIntTrig_t m_extIntTrigger;

        int m_number;

        bool m_isAnalog;
        bool m_isOut;
        bool m_outMask; // Pin always output
        bool m_inpMask; // Pin always input (inverted: 0 means always input)
        bool m_puMask;  // Pullup always on
        bool m_openColl;

        uint8_t m_pinMask;
};

#endif
