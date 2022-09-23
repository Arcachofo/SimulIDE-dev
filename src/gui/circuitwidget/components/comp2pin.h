/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMP2PIN_H
#define COMP2PIN_H

#include "component.h"

class MAINMODULE_EXPORT Comp2Pin : public Component
{
    public:
        Comp2Pin( QObject* parent, QString type, QString id );
        ~Comp2Pin(){}
};

#endif
