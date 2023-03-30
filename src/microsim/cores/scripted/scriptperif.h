/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTPERIF_H
#define SCRIPTPERIF_H

class ScriptCpu;
class asIScriptEngine;

class ScriptPerif
{
    public:
        ScriptPerif();
        ~ScriptPerif();

        virtual void registerScript( ScriptCpu* cpu ){;}
        virtual void startScript(){;}

    protected:
        ScriptCpu* m_scriptCpu;
};

#endif
