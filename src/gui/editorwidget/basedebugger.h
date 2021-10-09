/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef BASEDEBUGGER_H
#define BASEDEBUGGER_H

#include <QHash>

#include "compiler.h"

class BaseDebugger : public Compiler    // Base Class for all debuggers
{
        friend class McuInterface;

    Q_OBJECT
    public:
        BaseDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~BaseDebugger();

        virtual bool upload();

        void setLstType( int type ) { m_lstType = type; }
        void setLangLevel( int level ) { m_langLevel = level; }

        void setLineToFlash( int line, int addr );

        int getValidLine( int pc );

        QString getVarType( QString var );

        QStringList getVarList() { return m_varNames; }
        void setVarList( QStringList varNames ) { m_varNames = varNames; }

        QList<int> getSubLines() { return m_subLines; }
        
        bool m_stepOver;

    protected:
        virtual void getInfoInFile( QString line );
        virtual void preProcess() override;
        virtual bool postProcess() override;

        bool isNoValid( QString line );

        virtual void getSubs(){;}

        int m_processorType;
        int m_lastLine;
        int m_lstType;   // 0 doesn't use ":" (gpasm ), 1 uses ":" (avra, gavrasm)
        int m_langLevel; // 0 for asm, 1 for high level
        int m_codeStart;

        QString m_appPath;
        
        QStringList m_varNames;
        QStringList m_subs;
        QList<int>  m_subLines;
        
        QHash<QString, QString> m_typesList;
        QHash<QString, QString> m_varList;
        QHash<int, int> m_flashToSource;    // Map flash adress to Source code line
        QHash<int, int> m_sourceToFlash;    // Map .asm code line to flash adress

};

#endif

