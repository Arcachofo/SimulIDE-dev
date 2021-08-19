/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef RELAY_BASE_H
#define RELAY_BASE_H

#include "e-inductor.h"
#include "mech_contact.h"

class eNode;

class MAINMODULE_EXPORT RelayBase : public MechContact
{
    Q_OBJECT
    Q_PROPERTY( bool DT         READ dt       WRITE setDt    DESIGNABLE true USER true )
    Q_PROPERTY( double Rcoil    READ rCoil    WRITE setRCoil DESIGNABLE true USER true )
    Q_PROPERTY( double IOn      READ iTrig    WRITE setITrig DESIGNABLE true USER true )
    Q_PROPERTY( double IOff     READ iRel     WRITE setIRel  DESIGNABLE true USER true )
    Q_PROPERTY( double Inductance READ induc WRITE setInduc  DESIGNABLE true USER true )

    public:

        RelayBase( QObject* parent, QString type, QString id );
        ~RelayBase();

        double rCoil() const { return m_resistor->res(); }
        void setRCoil( double res ) { if( res > 0.0 ) m_resistor->setResSafe(res); }

        double iTrig() const { return m_trigCurrent; }
        void setITrig( double c ) { if( c > 0.0 ) m_trigCurrent = c; }

        double iRel() const { return m_relCurrent; }
        void setIRel( double current ) { m_relCurrent = current; }

        double induc() { return m_inductor->ind(); }
        void  setInduc( double i ) { m_inductor->setInd( i ); }

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void remove() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        eResistor* m_resistor;
        eInductor* m_inductor;

        eNode* m_internalEnode;
        double m_trigCurrent;
        double m_relCurrent;

        bool m_relayOn;
};

#endif
