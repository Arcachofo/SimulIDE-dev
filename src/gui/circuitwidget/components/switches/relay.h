/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef RELAY_H
#define RELAY_H

#include "mech_contact.h"
#include "inductor.h"
//#include "e-resistor.h"

class LibraryItem;
class eNode;

class MAINMODULE_EXPORT Relay : public MechContact
{
        Q_OBJECT
    public:
        Relay( QObject* parent, QString type, QString id );
        ~Relay();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double iTrig() { return m_trigCurrent; }
        void setITrig( double c ) { if( c > 0.0 ) m_trigCurrent = c; }

        double iRel() { return m_relCurrent; }
        void setIRel( double current ) { m_relCurrent = current; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        Inductor* m_inductor;

        eNode* m_internalEnode;

        double m_trigCurrent;
        double m_relCurrent;

        bool m_relayOn;
};

#endif
