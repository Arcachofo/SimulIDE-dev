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

#ifndef I2CTOPARALLEL_H
#define I2CTOPARALLEL_H

#include "twimodule.h"
#include "iocomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT I2CToParallel : public IoComponent, public TwiModule
{
    public:
        I2CToParallel( QObject* parent, QString type, QString id );
        ~I2CToParallel();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        int cCode() { return m_address; }
        void setCcode( int code ) { m_address = code; }
        
        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void writeByte();
        virtual void readByte();
};

#endif
