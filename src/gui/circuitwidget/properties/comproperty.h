/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPROPERTY_H
#define COMPROPERTY_H

#include <QString>

class PropVal;
class PropDialog;
class Component;

class MAINMODULE_EXPORT ComProperty
{
    public:
        ComProperty( QString name, QString caption, QString unit, QString type );
        virtual ~ComProperty(){;}

        QString name() { return m_name; }
        QString capt() { return m_capt; }
        QString type() { return m_type; }
        QString unit() { return m_unit; }

        virtual void    setValStr( QString val ){;}
        virtual QString getValStr(){return "";}
        virtual double  getValue(){return 0;}

        virtual QString toString(){return getValStr();}

        void setWidget( PropVal* w );

    protected:
        PropVal* m_widget;

        QString m_name;
        QString m_capt;
        QString m_type;
        QString m_unit;
        uint8_t m_flags;
};

#endif
