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

#ifndef STRINGPROP_H
#define STRINGPROP_H

#include "comproperty.h"
#include "stringval.h"
#include "enumval.h"

template <class Comp>
class MAINMODULE_EXPORT StringProp : public ComProperty
{
    public:
        StringProp( QString name, QString caption, QString unit, Comp* comp
                    , QString (Comp::*getter)(), void (Comp::*setter)(QString), QString type="string" )
        : ComProperty( name, caption, unit, type )
        {
            m_component = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~StringProp(){;}

        virtual void setValStr( QString val ) override
        {
            (m_component->*m_setter)( val );
            if( (m_component->itemType() == "Connector")
              || (m_component->itemType() == "Node")   ) return;
            Component* comp = (Component*)m_component;
            if( comp->showProp() == m_name ) comp->setValLabelText( val );
        }

        virtual QString getValStr() override
        { return (m_component->*m_getter)(); }

        virtual PropVal* getWidget( PropDialog* dialog ) override
        {
            if( !m_widget )
            {
                if( m_type == "enum") m_widget = new EnumVal( dialog, (Component*)m_component, this );
                else                  m_widget = new StringVal( dialog, (Component*)m_component, this );
            }
            return m_widget;
        }

    private:
        Comp* m_component;
        QString (Comp::*m_getter)();
        void    (Comp::*m_setter)(QString);
};

#endif
