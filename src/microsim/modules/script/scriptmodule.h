/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTMODULE_H
#define SCRIPTMODULE_H

#include "mcumodule.h"
#include "scriptbase.h"
//#include "e-element.h"


class ScriptModule : public ScriptBase, public McuModule //eElement
{
    public:
        ScriptModule( eMcu* mcu, QString name );
        ~ScriptModule();


    protected:

};
#endif
