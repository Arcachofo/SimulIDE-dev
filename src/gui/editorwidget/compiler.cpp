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
#include "mainwindow.h"
#include "utils.h"

Compiler::Compiler( QObject* parent, OutPanelText* outPane, QString filePath )
        : QObject( parent )
        , m_compProcess( NULL )
{
    m_outPane = outPane;

    m_file     = filePath;
    m_fileDir  = getFileDir( filePath );
    m_fileExt  = getFileExt( filePath );
    m_fileName = getBareName( filePath );
    m_firmware = "";

    clearCompiler();
}
Compiler::~Compiler( ){}

void Compiler::clearCompiler()
{
    m_toolPath.clear();
    m_inclPath.clear();
    m_command.clear();
    m_arguments.clear();
    m_argsDebug.clear();
}

QString Compiler::replaceData( QString str )
{
    QString filePath = addQuotes( m_file );
    QString inclPath = addQuotes( m_inclPath );

    str = str.replace( "$filePath", filePath )
             .replace( "$fileDir",  m_fileDir )
             .replace( "$fileName", m_fileName )
             .replace( "$fileExt",  m_fileExt )
             .replace( "$inclPath", inclPath );
    return str;
}
void Compiler::loadCompiler( QString file )
{
    clearCompiler();
    if( file.isEmpty() ) return;

    QDomDocument domDoc = fileToDomDoc( file, "Compiler::loadCompiler" );
    if( domDoc.isNull() )
    {
        m_outPane->appendLine( "Error: Compiler file not valid:\n"+file+"\n" );
        return;
    }
    QDomElement compiler = domDoc.documentElement();

    QString incPath = "";

    if( compiler.hasAttribute("name") )      m_compName  = compiler.attribute( "name" );
    if( compiler.hasAttribute("incPath") )   incPath     = compiler.attribute( "incDir" );
    if( !incPath.isEmpty() ) m_inclPath = incPath;

    QDomNode node = compiler.firstChild();
    while( !node.isNull() )
    {
        QDomElement step = node.toElement();
        if( step.tagName() == "step")
        {
            QString command   = step.attribute( "command" );
            QString arguments = replaceData( step.attribute( "arguments" ));
            QString argsDebug = replaceData( step.attribute( "argsDebug" ));
            if( argsDebug.isEmpty() ) argsDebug = arguments ;

            m_command.append( command );
            m_arguments.append( arguments);
            m_argsDebug.append( argsDebug );
        }
        node = node.nextSibling();
    }
    readSettings();
    m_outPane->appendLine( m_compName+" Compiler successfully loaded.\n" );
}

int Compiler::compile( bool debug )
{
    int error = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    for( int i=0; i<m_command.size(); ++i )
    {
        QString command = m_toolPath + m_command.at(i);
        if( !m_toolPath.isEmpty() && !QFile::exists( command ) )
        {
            toolChainNotFound();
            error = 1;
            break;
        }
        command = addQuotes( command );
        QString arguments = debug ? m_argsDebug.at(i) : m_arguments.at(i);
        error = runStep( command + arguments );
        if( error ) break;
    }
    if( error == 0 ) m_firmware = m_fileDir+m_fileName+".hex";
    QApplication::restoreOverrideCursor();
    return error;
}

int Compiler::runStep( QString fullCommand )
{
    int error = 0;
    m_outPane->appendLine( "\nExecuting:\n"+fullCommand+"\n" );
    m_compProcess.setWorkingDirectory( m_fileDir );
    m_compProcess.start( fullCommand  );
    m_compProcess.waitForFinished(-1);

    QString p_stderr = m_compProcess.readAllStandardError();
    QString p_stdout = m_compProcess.readAllStandardOutput();

    if( !p_stdout.isEmpty() )m_outPane->appendLine( p_stdout+"\n" );
    if( !p_stderr.isEmpty() )
    {
        m_outPane->appendLine( "ERROR OUTPUT:" );
        m_outPane->appendLine( p_stderr+"\n" );
        error = -1;
    }
    if     ( p_stdout.toLower().contains( QRegExp("\berror\b") )) error = -1;
    else if( p_stderr.toLower().contains( QRegExp("\berror\b") )) error = -1;
    return error;
}

QString Compiler::getPath( QString msg )
{
    QString path = QFileDialog::getExistingDirectory( NULL
                         , msg
                         , m_toolPath
                         , QFileDialog::ShowDirsOnly
                         | QFileDialog::DontResolveSymlinks);

    if( !path.isEmpty() && !path.endsWith("/") ) path += "/";
    return path;
}

void Compiler::getCompilerPath()
{
    QString path = getPath( tr("Select Compiler Toolchain directory") );
    if( !path.isEmpty() ) setCompilerPath( path );
}

void Compiler::setCompilerPath( QString path )
{
    m_toolPath = path;
    MainWindow::self()->settings()->setValue( m_compName+"_toolPath", m_toolPath );
}

void Compiler::getIncludePath()
{
    QString path = getPath( tr("Select Compiler Include directory") );
    if( !path.isEmpty() ) setIncludePath( path );
}

void Compiler::setIncludePath( QString path )
{
    m_inclPath = path;
    MainWindow::self()->settings()->setValue( m_compName+"_inclPath", m_inclPath );
}

void Compiler::toolChainNotFound()
{
    m_outPane->appendLine( tr(": ToolChain not found") );
    m_outPane->appendLine( tr("Set proper ToolChain Path")+"\n" );
}

void Compiler::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();

    QString prop = m_compName+"_toolPath";
    if( settings->contains( prop ) ) m_toolPath = settings->value( prop ).toString();
    prop = m_compName+"_inclPath";
    if( settings->contains( prop ) ) m_inclPath = settings->value( prop ).toString();
}
#include "moc_compiler.cpp"
