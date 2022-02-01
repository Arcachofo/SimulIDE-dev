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

#ifndef BCDBASE_H
#define BCDBASE_H

#include "logiccomponent.h"

class MAINMODULE_EXPORT BcdBase : public LogicComponent
{
    public:
        BcdBase( QObject* parent, QString type, QString id );
        ~BcdBase();

        bool tristate() { return true; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

    protected:
 static const uint8_t m_values[];

    uint8_t m_digit;
};

#endif
