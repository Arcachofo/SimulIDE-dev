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

class McuPort;
class eMcu;

class MAINMODULE_EXPORT McuPin : public IoPin
{
    friend class McuPort;
    friend class McuCreator;

    public:
        McuPin( McuPort* port, int i, QString id , Component* mcu );
        ~McuPin();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        virtual void setOutState( bool state, bool st=true ) override;
        virtual void setPortState( bool state );
        void setDirection( bool out );
        //virtual void setPullup( bool up ) override;
        void setPullupMask( bool up ) { m_puMask = up;}
        void setExtraSource( double vddAdmit, double gndAdmit );

    protected:
        QString m_id;

        McuPort* m_port;

        int m_number;

        bool m_isOut;
        bool m_outMask; // Pin always output
        bool m_inpMask; // Pin always input (inverted: 0 means always input)
        //bool m_pullup;
        bool m_puMask; // Pullup always on
        bool m_openColl;

        double digital_thre;

        uint8_t m_pinMask;
};

#endif
