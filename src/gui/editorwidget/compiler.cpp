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

Compiler::Compiler( CodeEditor* parent, OutPanelText* outPane )
        : QObject( parent )
        , m_compProcess( this )
{
    m_document = parent;
    m_outPane = outPane;

    m_file     = parent->getFilePath();
    m_fileDir  = getFileDir( m_file );
    m_fileExt  = getFileExt( m_file );
    m_fileName = getBareName( m_file );
    m_firmware = "";
    m_buildPath = m_fileDir;

    clearCompiler();
}
Compiler::~Compiler( ){}

void Compiler::clearCompiler()
{
    m_toolPath.clear();
    m_command.clear();
    m_arguments.clear();
    m_argsDebug.clear();
    m_device.clear();
    m_type.clear();
    m_useDevice = false;
}

QString Compiler::replaceData( QString str )
{
    QString filePath = addQuotes( m_file );
    QString inclPath = addQuotes( m_inclPath );

    str = str.replace( "$filePath", filePath )
             .replace( "$fileDir" , m_fileDir )
             .replace( "$fileName", m_fileName )
             .replace( "$fileExt" , m_fileExt )
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

    QString inclPath = "";

    if( compiler.hasAttribute("name") ) m_compName = compiler.attribute( "name" );

    if( compiler.hasAttribute("inclPath") ) inclPath = compiler.attribute( "inclPath" );
    if( !inclPath.isEmpty() ) m_inclPath = inclPath;

    if( compiler.hasAttribute("useDevice")
     && (compiler.attribute( "useDevice" ) == "true") ) m_useDevice = true;

    if( compiler.hasAttribute("type") ) m_type = compiler.attribute( "type" );

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

    getData();

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
        if( m_useDevice && arguments.contains("$device") )
        {
            if( m_device.isEmpty() )
            {
                m_outPane->appendLine( tr("Error: device not defined") );
                error = 1;
                break;
            }
            else arguments = arguments.replace( "$device", m_device );
        }
        error = runBuildStep( command + arguments );
        if( error ) break;
    }
    if( error == 0 )
    {
        m_fileList.clear();
        m_fileList.append( m_fileName+m_fileExt );
        m_firmware = m_fileDir+m_fileName+".hex";
    }
    QApplication::restoreOverrideCursor();
    return error;
}

int Compiler::runBuildStep( QString fullCommand )
{
    int error = 0;
    m_outPane->appendLine( "Executing:\n"+fullCommand+"\n" );
    m_compProcess.setWorkingDirectory( m_fileDir );
    m_compProcess.start( fullCommand  );
    m_compProcess.waitForFinished(-1);

    QString p_stderr = m_compProcess.readAllStandardError();
    QString p_stdout = m_compProcess.readAllStandardOutput();

    if( !p_stdout.isEmpty() )
    {
        m_outPane->appendLine( p_stdout+"\n" );
        p_stdout = p_stdout.toLower();
        if( p_stdout.contains( QRegExp("\\berror\\b") )) error = -1;
    }
    if( !p_stderr.isEmpty() )
    {
        QString stderr = p_stderr.toLower();
        if( stderr.contains( QRegExp("\\berror\\b") ))
        {
            m_outPane->appendLine( "ERROR OUTPUT:" );
            m_outPane->appendLine( p_stderr+"\n" );
            error = -1;
    }   }
    return error;
}

void Compiler::getDevice( QString line )
{
    line = line.toLower();
    line.remove(" ").remove("/");
    QStringList wordList= line.split( "=" );

    QString word = "";
    if( wordList.size() > 1 )
    {
        word = wordList.takeFirst();
        if( word != "device" && word != "board" ) return;

        word = wordList.takeFirst();
        if( word.isEmpty() ) return;

        m_device = word;
        m_document->setDevice( m_device );
}   }

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

void Compiler::getToolPath()
{
    QString path = getPath( tr("Select Compiler Toolchain directory") );
    if( !path.isEmpty() ) setToolPath( path );
}

void Compiler::setToolPath( QString path )
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
    if( settings->contains( prop ) ) setToolPath( settings->value( prop ).toString() );
    prop = m_compName+"_inclPath";
    if( settings->contains( prop ) ) m_inclPath = settings->value( prop ).toString();
}
#include "moc_compiler.cpp"
