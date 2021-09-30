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
#include "numval.h"

template <class Comp>
class MAINMODULE_EXPORT IntProp : public NumProp
{
    public:
        IntProp( QString name, QString caption, QString unit, Comp* comp
                   , int (Comp::*getter)(), void (Comp::*setter)(int), QString type="int" )
        : NumProp( name, caption, unit, type )
        {
            m_component = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~IntProp(){;}

        virtual void setUnit( QString u ) override  // Old: TODELETE
        {
            double multiplier = getMultiplier( u );
            QString val = QString::number( (m_component->*m_getter)()/multiplier );
            setValStr( val+" "+u );
        }

        virtual void setValStr( QString val ) override
        {
            QStringList l = val.split(" ");
            double  v = l.first().toDouble();
            val = QString::number( v );
            if( l.size() > 1 ) m_unit = l.last();
            v = v*getMultiplier( m_unit );

            (m_component->*m_setter)( (int)v );
            if( m_component->itemType() == "Label" ) return;
            Component* comp = (Component*)m_component;
            if( comp->showProp() == m_name ) comp->setValLabelText( val );
        }

        virtual QString getValStr()  override
        { return getStr( (m_component->*m_getter)() ); }

        virtual QString getValue()
        { return QString::number( (m_component->*m_getter)() ); }

        virtual PropVal* getWidget( PropDialog* dialog ) override
        {
            if( !m_widget ) m_widget = new NumVal( dialog, (Component*)m_component, this );
            return m_widget;
        }

    private:
        Comp* m_component;
        int  (Comp::*m_getter)();
        void (Comp::*m_setter)(int);
};

#endif
