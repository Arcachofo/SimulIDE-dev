/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DOUBPROP_H
#define DOUBPROP_H

#include "comproperty.h"
#include "utils.h"
#include "compbase.h"

class CompBase;

template <class Comp>
class DoubProp : public ComProperty
{
    public:
        DoubProp( QString name, QString caption, QString unit, Comp* comp
                , double (Comp::*getter)(), void (Comp::*setter)(double)
                , uint8_t flags=0, QString type="double" )
        : ComProperty( name, caption, unit, type, flags )
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

        double getVal( const QString &val, CompBase* comp )
        {
            QStringList l = val.split(" ");
            double  v = l.first().toDouble();

            if( l.size() > 1 )
            {
                QString unit = l.last();
                if( !unit.startsWith("_") ) m_unit = unit;
            }
            if( comp->getPropStr("ShowProp") == m_name ) comp->setPropStr("ValLabelText", QString::number( v )+" "+m_unit );

            return  v*getMultiplier( m_unit );
        }

        QString getStr( double val )
        {
            double multiplier = getMultiplier( m_unit );
            QString  valStr = QString::number( val/multiplier );
            if( !m_unit.isEmpty() ) valStr.append(" "+m_unit );
            return valStr;
        }
};

#endif
