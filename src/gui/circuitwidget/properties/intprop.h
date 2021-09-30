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

#ifndef INTPROP_H
#define INTPROP_H

#include "numprop.h"

template <class Comp>
class MAINMODULE_EXPORT IntProp : public NumProp
{
    public:
        IntProp( QString name, QString caption, QString unit, Comp* comp
               , int (Comp::*getter)(), void (Comp::*setter)(int), QString type="int" )
        : NumProp( name, caption, unit, type )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~IntProp(){;}

        virtual void setUnit( QString u ) override  // Old: TODELETE
        { setValStr( getValU( (m_comp->*m_getter)(), u ) ); }

        virtual void setValStr( QString val ) override
        { (m_comp->*m_setter)( getVal( val, m_comp ) ); }

        virtual QString getValStr()  override
        { return getStr( (m_comp->*m_getter)() ); }

        virtual double getValue()
        { return (m_comp->*m_getter)(); }

    private:
        Comp* m_comp;
        int  (Comp::*m_getter)();
        void (Comp::*m_setter)(int);
};

#endif
