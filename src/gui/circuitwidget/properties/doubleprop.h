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

#ifndef DOUBPROP_H
#define DOUBPROP_H

#include "numprop.h"
#include "numval.h"

template <class Comp>
class MAINMODULE_EXPORT DoubProp : public NumProp
{
    public:
        DoubProp( QString name, QString caption, QString unit, Comp* comp
                , double (Comp::*getter)(), void (Comp::*setter)(double), QString type="double" )
        : NumProp( name, caption, unit, type )
        {
            m_component = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~DoubProp(){;}

        virtual void setValStr( QString val ) override
        {
            (m_component->*m_setter)( getVal( val ) );
            if( m_component->showProp() == m_name ) m_component->setValLabelText( val );
        }

        virtual QString getValStr() override
        { return getStr( (m_component->*m_getter)() ); }

        virtual QString getValue()
        { return QString::number( (m_component->*m_getter)() ); }

        virtual PropVal* getWidget( PropDialog* dialog ) override
        {
            if( !m_widget ) m_widget = new NumVal( dialog, m_component, this );
            return m_widget;
        }

    private:
        Comp* m_component;
        double (Comp::*m_getter)();
        void   (Comp::*m_setter)(double);
};

#endif
