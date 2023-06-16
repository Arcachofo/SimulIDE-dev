/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DOUBPROP_H
#define DOUBPROP_H

#include "numprop.h"

template <class Comp>
class MAINMODULE_EXPORT DoubProp : public NumProp
{
    public:
        DoubProp( QString name, QString caption, QString unit, Comp* comp
                , double (Comp::*getter)(), void (Comp::*setter)(double)
                , uint8_t flags=0, QString type="double" )
        : NumProp( name, caption, unit, type, flags )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~DoubProp(){;}

        virtual void setValStr( QString val ) override
        { (m_comp->*m_setter)( getVal( val, m_comp ) ); }

        virtual QString getValStr() override
        { return getStr( (m_comp->*m_getter)() ); }

        virtual double getValue()
        { return (m_comp->*m_getter)(); }

    private:
        Comp* m_comp;
        double (Comp::*m_getter)();
        void   (Comp::*m_setter)(double);
};

#endif
