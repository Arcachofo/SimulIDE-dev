/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef MECH_CONTACT_H
#define MECH_CONTACT_H

#include "component.h"
#include "e-resistor.h"

class MAINMODULE_EXPORT MechContact : public Component, public eElement
{
    public:
        MechContact( QObject* parent, QString type, QString id );
        ~MechContact();

        virtual int poles() { return m_numPoles; }
        virtual void setPoles( int poles );

        virtual bool dt() { return (m_numthrows>1); }
        virtual void setDt( bool dt );
        
        virtual bool nClose() { return m_nClose; }
        virtual void setNClose( bool nc );

        virtual void stamp() override;
        virtual void remove() override;
        virtual void updateStep() override { if( m_changed ) { m_changed = false; update(); } }

        void  SetupSwitches( int poles, int throws );
        void  SetupButton();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        virtual void setSwitch( bool on );

        std::vector<eResistor*> m_switches;

        bool m_closed;
        bool m_nClose;
        bool m_ButHidden;

        int m_numPoles;
        int m_numthrows;

        int m_pin0;
};

#endif
