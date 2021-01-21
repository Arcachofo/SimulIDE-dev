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

#include <QDomDocument>

#include "compiler.h"
#include "editorwindow.h"
#include "outpaneltext.h"
#include "utils.h"

Compiler::Compiler( EditorWindow* parent, OutPanelText* outPane )
        : QObject( parent )
        , m_compProcess( NULL )
{
    m_editor  = parent;
    m_outPane = outPane;

    clearCompiler();
}
Compiler::~Compiler( )
{
}

void Compiler::clearCompiler()
{
    m_toolChain = false;
    m_toolPath.clear();
    m_command.clear();
    m_arguments.clear();
}

void Compiler::loadCompiler( QString file )
{
    QDomDocument domDoc = fileToDomDoc( file, "Compiler::loadCompiler" );
    if( domDoc.isNull() )
    {
        m_outPane->writeText( "Error: Compiler file not valid:\n"+file+"\n" );
        return;
    }
    QDomElement root = domDoc.documentElement();

    QString compName = "";

    if( root.hasAttribute("name") )  compName = root.attribute( "name" );
    if( root.hasAttribute("toolPath") )  m_toolPath = root.attribute( "toolPath" );
    if( root.hasAttribute("command") )   m_command  = root.attribute( "command" );
    if( root.hasAttribute("arguments") ) m_arguments = root.attribute( "arguments" );

    if( !QFile::exists( m_toolPath+m_command ) )
    {
        m_outPane->appendText( tr("Error: ToolChain not found")+"\n" );
        m_outPane->writeText( m_toolPath+m_command+"\n" );
        return;
    }
    m_outPane->writeText( compName+" Compiler successfully loaded.\n" );
    m_toolChain = true;
}

int Compiler::compile( QString file )
{
    if( !m_toolChain ) return -1;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    int error = 0;

    QString command = m_toolPath+m_command;

    #ifndef Q_OS_UNIX
    command  = addQuotes( command );
    file     = addQuotes( file );
    #endif

    QString arguments = m_arguments.replace( "$Fi", file );

    m_outPane->writeText( "\n Executing:\n"+command+arguments+"\n" );
    m_compProcess.start( command+arguments  );
    m_compProcess.waitForFinished(-1);

    QString p_stderr = m_compProcess.readAllStandardError();
    QString p_stdout = m_compProcess.readAllStandardOutput();

    m_outPane->writeText( p_stdout+"\n" );
    if( !p_stderr.isEmpty() )
    {
        m_outPane->writeText( "ERROR OUTPUT:\n" );
        m_outPane->writeText( p_stderr+"\n" );
    }

    QApplication::restoreOverrideCursor();
    return error;
}


#include "moc_compiler.cpp"

