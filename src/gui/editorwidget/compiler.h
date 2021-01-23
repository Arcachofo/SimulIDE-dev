/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef COMPILER_H
#define COMPILER_H

#include <QtGui>

class EditorWindow;
class OutPanelText;

class Compiler : public QObject
{
    Q_OBJECT

    public:
        Compiler( QObject* parent, OutPanelText* outPane );
        ~Compiler();

        void clearCompiler();
        void loadCompiler( QString file );
        int compile( QString file );

    protected:

        bool m_toolChain;

        QString m_toolPath;
        QString m_command;
        QString m_arguments;

        QProcess m_compilerProc;

        //EditorWindow* m_editor;
        OutPanelText* m_outPane;
};

#endif

