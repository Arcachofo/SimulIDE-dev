/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INTPROP_H
#define INTPROP_H

#include "comproperty.h"
#include "utils.h"
#include "compbase.h"

class CompBase;

template <class Comp>
class IntProp : public ComProperty
{
    public:
        IntProp( QString name, QString caption, QString unit, Comp* comp
               , int (Comp::*getter)(), void (Comp::*setter)(int)
               , uint8_t flags=0, QString type="int" )
        : ComProperty( name, caption, unit, type, flags )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~IntProp(){;}

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

        int getVal( const QString &val, CompBase* comp )
        {
            QStringList l = val.split(" ");
            int v = l.first().toInt();

            if( l.size() > 1 )
            {
                QString unit = l.last();
                if( !unit.startsWith("_") ) m_unit = unit;
            }
            if( comp->getPropStr("ShowProp") == m_name ) comp->setPropStr("ValLabelText", QString::number( v )+" "+m_unit );

            return  v*getMultiplier( m_unit );
        }
        QString getStr( int val )
        {
            int multiplier = getMultiplier( m_unit );
            QString valStr = QString::number( val/multiplier );
            if( !m_unit.isEmpty() ) valStr.append(" "+m_unit );
            return valStr;
        }
};

#endif
