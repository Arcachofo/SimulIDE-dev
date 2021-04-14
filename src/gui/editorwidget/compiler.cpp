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

Compiler::Compiler( QObject* parent, OutPanelText* outPane )
        : QObject( parent )
        , m_compilerProc( NULL )
{
    //m_editor  = parent;
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
        m_outPane->appendLine( "Error: Compiler file not valid:\n"+file+"\n" );
        return;
    }
    QDomElement root = domDoc.documentElement();

    QString compName = "";
    QString incDir = "";

    if( root.hasAttribute("name") )      compName    = root.attribute( "name" );
    if( root.hasAttribute("toolPath") )  m_toolPath  = root.attribute( "toolPath" );
    if( root.hasAttribute("command") )   m_command   = root.attribute( "command" );
    if( root.hasAttribute("arguments") ) m_arguments = root.attribute( "arguments" );
    if( root.hasAttribute("incDir") )    incDir      = root.attribute( "incDir" );

    if( !incDir.isEmpty() ) m_incDir = incDir;

    if( !m_toolPath.isEmpty() && !QFile::exists( m_toolPath+m_command ) )
    {
        m_outPane->appendLine( tr("Error: ToolChain not found") );
        m_outPane->appendLine( m_toolPath+m_command+"\n" );
        m_outPane->appendLine( "Please set Compiler path in file:\n"+file+"\n" );
        return;
    }
    m_outPane->appendLine( compName+" Compiler successfully loaded.\n" );
    m_toolChain = true;
}

int Compiler::compile( QString file )
{
    if( !m_toolChain ) return -1;

    QApplication::setOverrideCursor( Qt::WaitCursor );
    int error = 0;

    QFileInfo fi = QFileInfo( file );
    QString filePath = file;
    QString fileDir  = fi.absolutePath()+"/";
    QString fileExt  = "."+fi.suffix();
    QString fileName = fi.completeBaseName();
    QString incDir   = m_incDir;

    QString command = m_toolPath+m_command;

    #ifndef Q_OS_UNIX
    command  = addQuotes( command );
    filePath = addQuotes( filePath );
    fileDir  = addQuotes( fileDir );
    fileExt  = addQuotes( fileExt );
    fileName = addQuotes( fileName );
    incDir   = addQuotes( incDir );
    #endif

    QString arguments = m_arguments.replace( "$filePath", filePath )
                                   .replace( "$fileDir",  fileDir )
                                   .replace( "$fileName", fileName )
                                   .replace( "$fileExt",  fileExt )
                                   .replace( "$incDir",   incDir );

    m_outPane->appendLine( "\n Executing:\n"+command+arguments+"\n" );
    m_compilerProc.start( command+arguments  );
    m_compilerProc.waitForFinished(-1);

    QString p_stderr = m_compilerProc.readAllStandardError();
    QString p_stdout = m_compilerProc.readAllStandardOutput();

    if     ( p_stdout.toLower().contains( QRegExp("\berror\b") )) error = -1;
    else if( p_stderr.toLower().contains( QRegExp("\berror\b") )) error = -1;

    m_outPane->appendLine( p_stdout+"\n" );
    if( !p_stderr.isEmpty() )
    {
        m_outPane->appendLine( "ERROR OUTPUT:" );
        m_outPane->appendLine( p_stderr+"\n" );
    }

    QApplication::restoreOverrideCursor();
    return error;
}

#include "moc_compiler.cpp"

