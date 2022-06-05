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

#include "strprop.h"

template <class Comp>
class MAINMODULE_EXPORT StringProp : public StrProp
{
    public:
        StringProp( QString name, QString caption, QString unit, Comp* comp
                  , QString (Comp::*getter)(), void (Comp::*setter)(QString), QString type="string" )
        : StrProp( name, caption, unit, type )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~StringProp(){;}

        virtual void setValStr( QString val ) override
        {
            if( m_comp->showProp() == m_name ) m_comp->setValLabelText( val );
            (m_comp->*m_setter)( setStr( val ) ); // Comp setter can change valLabel
        }

        virtual QString getValStr() override
        { return (m_comp->*m_getter)(); }

        virtual QString toString(){ return getStr( (m_comp->*m_getter)() ); }

    private:
        Comp* m_comp;
        QString (Comp::*m_getter)();
        void    (Comp::*m_setter)(QString);
};

#endif
