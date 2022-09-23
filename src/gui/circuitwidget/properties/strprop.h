/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef STRPROP_H
#define STRPROP_H

#include "comproperty.h"

class CompBase;

class MAINMODULE_EXPORT StrProp : public ComProperty
{
    public:
        StrProp( QString name, QString caption, QString unit, QString type )
        : ComProperty( name, caption, unit, type )
        {}
        ~StrProp(){;}

    protected:
        QString getStr( QString str );
        QString setStr( QString str );
};

#endif
