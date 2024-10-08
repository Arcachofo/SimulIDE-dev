/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTSUBC_H
#define SCRIPTSUBC_H

#include "mcu.h"
#include "embedcircuit.h"

class ScriptSubc : public Mcu, public EmbedCircuit
{
    public:
        ScriptSubc( QString type, QString id );
        ~ScriptSubc();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

    private:

};

#endif
