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

        virtual void setState( bool state, bool st=false ) override;
        virtual void setPortState( bool state );
        void setDirection( bool out );
        void setPullup( bool up );
        void setPullupMask( bool up ) { m_puMask = up;}
        void setExtraSource( double vddAdmit, double gndAdmit );

        virtual bool getState() override;

        Pin* pin() const { return ( static_cast<Pin*>(m_ePin[0]) ); }

    protected:
        QString m_id;

        McuPort* m_port;

        int m_number;

        bool m_outState;
        bool m_inState;
        bool m_isOut;
        bool m_dirMask; // Pin always output
        bool m_pullup;
        bool m_puMask; // Pullup always on
        bool m_openColl;

        uint8_t m_pinMask;
};

#endif
