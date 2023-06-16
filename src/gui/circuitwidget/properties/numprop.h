/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef NUMPROP_H
#define NUMPROP_H

#include "comproperty.h"

class CompBase;

class MAINMODULE_EXPORT NumProp : public ComProperty
{
    public:
        NumProp( QString name, QString caption, QString unit, QString type, uint8_t flags )
        : ComProperty( name, caption, unit, type, flags )
        {}
        ~NumProp(){;}

    protected:
        const QString getValU( double val, QString &u );

        double getVal( const QString &val, CompBase* comp );

        QString getStr( double val );
};

#endif
