/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef STRBASEPROP_H
#define STRBASEPROP_H

#include "comproperty.h"

class CompBase;

class MAINMODULE_EXPORT StrBaseProp : public ComProperty
{
    public:
        StrBaseProp( QString name, QString caption, QString unit, QString type, uint8_t flags )
        : ComProperty( name, caption, unit, type, flags )
        {}
        ~StrBaseProp(){;}

        virtual double getValue() override;

    protected:
        QString getStr( QString str );
        QString setStr( QString str );
};

#endif
