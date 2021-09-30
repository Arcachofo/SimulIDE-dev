/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef BOOLPROP_H
#define BOOLPROP_H

#include "comproperty.h"

template <class Comp>
class MAINMODULE_EXPORT BoolProp : public ComProperty
{
    public:
        BoolProp( QString name, QString caption, QString unit, Comp* comp
                , bool (Comp::*getter)(), void (Comp::*setter)(bool), QString type="bool" )
        : ComProperty( name, caption, unit, type )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~BoolProp(){;}

        virtual void setValStr( QString val ) override
        { (m_comp->*m_setter)( val == "true" ); }

        virtual QString getValStr() override
        { return (m_comp->*m_getter)() ? "true" : "false"; }

    private:
        Comp* m_comp;
        bool (Comp::*m_getter)();
        void (Comp::*m_setter)(bool);
};

#endif
