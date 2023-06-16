/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPROPERTY_H
#define COMPROPERTY_H

#include <QString>

enum propFlags{
    propHidden = 1,
    propNoCopy = 1<<1,
};

class MAINMODULE_EXPORT ComProperty
{
    public:
        ComProperty( QString name, QString caption, QString unit, QString type, uint8_t flags );
        virtual ~ComProperty(){;}

        QString name();
        QString capt();
        QString type();
        QString unit();
        uint8_t flags();

        virtual void    setValStr( QString );
        virtual QString getValStr();
        virtual double  getValue();

        virtual QString toString();

        //void setWidget( PropVal* w );

    protected:
        //PropVal* m_widget;

        QString m_name;
        QString m_capt;
        QString m_type;
        QString m_unit;
        uint8_t m_flags;
};

#endif
