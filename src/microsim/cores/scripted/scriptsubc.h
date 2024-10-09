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

    protected:
        virtual Pin* addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8, int space=0 ) override;

};

#endif
