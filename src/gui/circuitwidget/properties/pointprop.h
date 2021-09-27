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

#ifndef POINTPROP_H
#define POINTPROP_H

#include <QPointF>

#include "comproperty.h"
#include "utils.h"

template <class Comp>
class MAINMODULE_EXPORT PointProp : public ComProperty
{
    public:
        PointProp( QString name, QString caption, QString unit, Comp* comp
                  , QPointF (Comp::*getter)(), void (Comp::*setter)(QPointF), QString type="point" )
        : ComProperty( name, caption, unit, type )
        {
            m_component = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~PointProp(){;}

        virtual void setValStr( QString val ) override
        { (m_component->*m_setter)( getPointF( val ) ); }

        virtual QString getValStr()  override
        { return getStrPointF( (m_component->*m_getter)() ); }

    private:
        Comp* m_component;
        QPointF (Comp::*m_getter)();
        void    (Comp::*m_setter)(QPointF);
};

#endif
