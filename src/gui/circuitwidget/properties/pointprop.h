/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef POINTPROP_H
#define POINTPROP_H

#include <QPointF>

#include "comproperty.h"
#include "proputils.h"

template <class Comp>
class MAINMODULE_EXPORT PointProp : public ComProperty
{
    public:
        PointProp( QString name, QString caption, QString unit, Comp* comp
                 , QPointF (Comp::*getter)(), void (Comp::*setter)(QPointF)
                 , QString type="point", uint8_t flags=0 )
        : ComProperty( name, caption, unit, type, flags )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~PointProp(){;}

        virtual void setValStr( QString val ) override
        { (m_comp->*m_setter)( getPointF( val ) ); }

        virtual QString getValStr()  override
        { return getStrPointF( (m_comp->*m_getter)() ); }

    private:
        Comp* m_comp;
        QPointF (Comp::*m_getter)();
        void    (Comp::*m_setter)(QPointF);
};

#endif
