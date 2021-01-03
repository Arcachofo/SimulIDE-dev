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

#include "e-source.h"
#include "pin.h"

class McuPort;
class eMcu;

class MAINMODULE_EXPORT McuPin : public eSource
{
    friend class McuPort;
    friend class McuCreator;

    public:
        McuPin( McuPort* port, int i, QString id , Component* mcu );
        ~McuPin();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        void controlPin( bool ctrl );
        void setState( bool state );
        void setPortState( bool state );
        void setDirection( bool out );
        void setPullup( bool up );
        void setPullupMask( bool up ) { m_puMask = up;}

        bool getState() { return m_state; }

        Pin* pin() const { return ( static_cast<Pin*>(m_ePin[0]) ); }

    protected:
        void update();

        QString m_id;

        McuPort* m_port;

        int m_number;

        bool m_state;
        bool m_isOut;
        bool m_dirMask; // Pin always output
        bool m_extCtrl;
        bool m_pullup;
        bool m_puMask; // Pullup always on
        bool m_openColl;

        double m_vddAdmit;  // Out stage
        double m_gndAdmit;  // Out Stage
        double m_vddAdmEx;  // Extra Source (vref out)
        double m_gndAdmEx;  // Extra Source (vref out)
        double m_pupAdmit;  // Internal Pullup

        double m_volt;

        uint8_t m_pinMask;

        //uint16_t m_lastTime;
};

#endif
