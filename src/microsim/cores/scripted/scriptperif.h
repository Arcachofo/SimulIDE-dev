/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTPERIF_H
#define SCRIPTPERIF_H

#include <QString>
#include <QStringList>

class ScriptCpu;
class asIScriptEngine;

class ScriptPerif
{
    public:
        ScriptPerif( QString name );
        ~ScriptPerif();

        virtual QStringList registerScript( ScriptCpu* cpu ){;}
        virtual void startScript(){;}

        QString type() { return m_type; }

    protected:
        QString m_perifName;

        ScriptCpu* m_scriptCpu;

        QString m_type;

        QStringList m_methods;
};

#endif
