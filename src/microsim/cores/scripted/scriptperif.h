/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTPERIF_H
#define SCRIPTPERIF_H

#include <QString>

class ScriptCpu;
class asIScriptEngine;

class ScriptPerif
{
    public:
        ScriptPerif( QString name );
        ~ScriptPerif();

        virtual void registerScript( ScriptCpu* cpu ){;}
        virtual void startScript(){;}

    protected:
        QString m_perifName;

        ScriptCpu* m_scriptCpu;
};

#endif
