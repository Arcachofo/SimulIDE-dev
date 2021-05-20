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

#ifndef BCDTODEC_H
#define BCDTODEC_H

#include "logiccomponent.h"
#include "e-element.h"

class LibraryItem;

class MAINMODULE_EXPORT BcdToDec : public LogicComponent, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool Inverted READ invertOuts WRITE setInvertOuts DESIGNABLE true USER true )
    Q_PROPERTY( bool _16_Bits READ _16bits    WRITE set_16bits    DESIGNABLE true USER true )

    public:
        BcdToDec( QObject* parent, QString type, QString id );
        ~BcdToDec();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }


        bool _16bits() { return m_16Bits; }
        void set_16bits( bool set );

   private:
        bool m_16Bits;

};

#endif

