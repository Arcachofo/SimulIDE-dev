/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef STRPROP_H
#define STRPROP_H

#include "strprop.h"

template <class Comp>
class MAINMODULE_EXPORT StrProp : public StrBaseProp
{
    public:
        StrProp( QString name, QString caption, QString unit, Comp* comp
               , QString (Comp::*getter)(), void (Comp::*setter)(QString)
               , uint8_t flags=0, QString type="string" )
        : StrBaseProp( name, caption, unit, type, flags )
        {
            m_comp = comp;
            m_getter = getter;
            m_setter = setter;
        }
        ~StrProp(){;}

        virtual void setValStr( QString val ) override
        {
            /// if( m_comp->showProp() == m_name ) m_comp->setValLabelText( val );  // Needed???
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
