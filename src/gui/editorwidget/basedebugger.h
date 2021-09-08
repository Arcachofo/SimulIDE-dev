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

class CodeEditor;

class BaseDebugger : public Compiler    // Base Class for all debuggers
{
        friend class McuInterface;

    Q_OBJECT
    public:
        BaseDebugger( CodeEditor* parent, OutPanelText* outPane, QString filePath );
        ~BaseDebugger();

        virtual bool upload();

        virtual void mapFlashToSource();

        virtual int getValidLine( int pc );

        virtual QString getVarType( QString var );
        virtual QStringList getVarList() { return m_varNames; }
        virtual QList<int> getSubLines() { return m_subLines; }
        
        int type;

    protected:
        virtual void getSubs(){;}
        virtual void getData(){;}

        void mapFlashToAvrGcc();
        void getAvrGccData();

        CodeEditor* m_editor;

        int m_processorType;
        int m_lastLine;

        QString m_appPath;
        
        QStringList m_varNames;
        QStringList m_subs;
        QList<int>  m_subLines;
        
        QHash<QString, QString> m_typesList;
        QHash<QString, QString> m_varList;
        QHash<int, int> m_flashToSource;            // Map flash adress to Source code line
        QHash<int, int> m_sourceToFlash;            // Map .asm code line to flash adress

};

#endif

