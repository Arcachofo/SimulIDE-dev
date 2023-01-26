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

#include <QApplication>
#include <QSettings>

#include "inodebugger.h"
#include "outpaneltext.h"
#include "mainwindow.h"
#include "circuit.h"
#include "utils.h"

static const char* InoDebugger_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Board"),
    QT_TRANSLATE_NOOP("App::Property","Custom Board")
};

InoDebugger::InoDebugger( CodeEditor* parent, OutPanelText* outPane )
           : AvrGccDebugger( parent, outPane )
{
    Q_UNUSED( InoDebugger_properties );

    m_version = 0;
    m_Ardboard = Uno;
    m_ArdboardList << "uno" << "megaADK" << "nano" << "diecimila" << "leonardo" << "custom";
    m_buildPath = MainWindow::self()->getFilePath("data/codeeditor/buildIno");
}
InoDebugger::~InoDebugger() {}

void InoDebugger::setToolPath( QString path )
{
    QString builder = "arduino-builder";
    #ifndef Q_OS_UNIX
    builder += ".exe";
    #endif
    if( QFile::exists( path+builder) )
    {
        m_version = 1;
        m_toolPath = path+"hardware/tools/avr/bin/";
        m_outPane->appendLine( "Found Arduino Version 1" );
    }
    else{
        builder = "resources/app/node_modules/arduino-ide-extension/build/arduino-cli";
        #ifndef Q_OS_UNIX
        builder += ".exe";
        #endif
        if( QFile::exists( path+builder) )
        {
            m_version = 2;
            QString command = path+builder;
            command = addQuotes( command );
            command += " config dump";

            QProcess getConfig( this );  // Get config
            getConfig.start( command );
            getConfig.waitForFinished();
            QString config = getConfig.readAllStandardOutput();
            getConfig.close();

            QStringList configLines = config.split( "\n" );
            QRegExp rx( "^  data: .*$" );
            int idx = configLines.indexOf(rx);
            if( idx != -1 )
            {
                m_toolPath = QDir::fromNativeSeparators( configLines[idx].mid( 8 ) )+"/packages/arduino/tools/avr-gcc/";
                QDir toolDir( m_toolPath );
                QStringList dirList = toolDir.entryList( QDir::Dirs, QDir::Name | QDir::Reversed );
                if( !dirList.isEmpty() ) m_toolPath += dirList[0]+"/bin/";
            }
            m_outPane->appendLine( "Found Arduino Version 2" );
        }
        else{                                          // Executable not found
            m_outPane->appendText( "\nArduino" );
            toolChainNotFound();
            return;
        }
    }
    m_builder = builder;
    m_arduinoPath = path;
    MainWindow::self()->settings()->setValue( m_compName+"_toolPath", path );
}

bool InoDebugger::upload() // Copy hex file to Circuit folder, then upload
{
    QString circDir  = Circuit::self()->getFilePath();
    QString firmPath =  m_firmware;

    QDir circuitDir = QFileInfo( circDir ).absoluteDir();
    m_firmware = circuitDir.absolutePath()+"/"+m_fileName+".hex";
    circuitDir.remove( m_fileName+".hex" );
    QFile::copy( firmPath, m_firmware );

    return BaseDebugger::upload();
}

int InoDebugger::compile( bool )
{
    if( m_version == 0 ) return -1;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString cBuildPath = m_buildPath+"/build";
    QString cCachePath = m_buildPath+"/cache";

    QDir dir( m_buildPath );
    bool b = dir.cd( "build" );
    if( b ) dir.removeRecursively(); // Remove old files
    dir.mkpath( cCachePath );  // Create cache folder ( if doesn't exist )
    dir.mkpath( cBuildPath );  // Create build folder

    if( !QFile::exists( cBuildPath ) || !QFile::exists( cCachePath ) )
    {
        m_outPane->appendLine( "\n    ERROR: Build folders NOT found at:\n    "+m_buildPath );
        QApplication::restoreOverrideCursor();
        return -1;
    }
    cBuildPath = addQuotes( cBuildPath );
    cCachePath = addQuotes( cCachePath );

    QString boardSource;
    QString boardName = getBoard();
    if( boardName.isEmpty() ){
        if( m_Ardboard == Custom ) boardSource = "Custom ";
        else                       boardSource = "Arduino";
    }else                          boardSource = "In File";

    if( m_Ardboard < Custom ) boardName = "arduino:avr:"+m_ArdboardList.at( m_Ardboard );
    else                      boardName = m_customBoard;

    QString command = addQuotes( m_arduinoPath+m_builder );
    if( m_version == 1 )
    {
        QString Scommand = m_arduinoPath+"arduino";
        if( m_sketchBook.isEmpty() ) // Find sketchBook
        {
            #ifndef Q_OS_UNIX
            Scommand += "_debug.exe";
            #endif
            Scommand = addQuotes( Scommand );
            Scommand += " --get-pref sketchbook.path";

            QProcess getSkBook( this );  // Get sketchBook Path
            getSkBook.start( Scommand );
            getSkBook.waitForFinished();
            m_sketchBook = getSkBook.readAllStandardOutput();
            m_sketchBook = m_sketchBook.remove("\r").remove("\n");
            getSkBook.close();
        }
        QString hardware   = addQuotes( m_arduinoPath+"hardware" );
        QString toolsBuild = addQuotes( m_arduinoPath+"tools-builder" );
        QString toolsAvr   = addQuotes( m_arduinoPath+"hardware/tools/avr" );
        QString libraries  = addQuotes( m_arduinoPath+"libraries" );
        QString userLibrar = addQuotes( m_sketchBook+"/libraries" );

        command += " -compile";
        command += " -hardware "+hardware;
        command += " -tools "+toolsBuild;
        command += " -tools "+toolsAvr;
        command += " -built-in-libraries "+libraries;
        command += " -libraries "+userLibrar;
        command += " -fqbn="+boardName;
        command += " -build-path "+cBuildPath;
        command += " -build-cache "+cCachePath;
    }
    else if( m_version == 2 )
    {
        command += " compile";
        command += " --no-color";
        command += " --fqbn="+boardName;
        command += " --build-path "+cBuildPath;
        command += " --build-cache-path "+cCachePath;
    }
    command += " "+addQuotes( m_file );
    m_firmware = "";

    m_outPane->appendLine( "\nExecuting:\n"+command+"\n" );
    m_compProcess.start( command );
    m_compProcess.waitForFinished(-1);

    m_outPane->appendLine( "Build folder: "+m_buildPath );
    m_outPane->appendLine( "SketchBook:   "+m_sketchBook );
    m_outPane->appendLine( boardSource+" Board "+addQuotes( boardName ) );
    m_outPane->appendLine( "" );

    QString p_stderr = m_compProcess.readAllStandardError();
    QString p_stdout = m_compProcess.readAllStandardOutput();
    if( !p_stdout.isEmpty() ) m_outPane->appendLine( p_stdout );

    int error = 0;
    if( !p_stderr.isEmpty() ) error = getError( p_stderr )+1;
    else{
        m_fileList.clear();
        m_fileList.append( m_fileName+m_fileExt );
        m_firmware = m_buildPath+"/build/"+m_fileName+".ino.hex";
        m_outPane->appendLine( "\n"+tr("     SUCCESS!!! Compilation Ok")+"\n" );
    }
    QApplication::restoreOverrideCursor();

    return error;
}

bool InoDebugger::postProcess()
{
    QString oldBuildPath = m_buildPath;
    QString oldFileName  = m_fileName;

    m_buildPath = m_buildPath+"/build/";
    m_fileName  = m_fileName+".ino";

    bool ok = AvrGccDebugger::postProcess();

    m_buildPath = oldBuildPath;
    m_fileName  = oldFileName;
    return ok;
}

QString InoDebugger::getBoard()
{
    QString board = m_board.toLower();
    if( board.isEmpty() ) return board;

    if( board == "duemilanove" ) board = "diecimila";
    else if( board == "mega" )   board = "megaADK";

    if( !m_ArdboardList.contains( board ) )
    {
        m_customBoard = m_board;
        m_Ardboard    = Custom;
    }
    else m_Ardboard = (board_t) m_ArdboardList.indexOf( board);

    return board;
}

void InoDebugger::setBoardName( QString board )
{
    m_board = board;
    getBoard();
    if( m_Ardboard ) m_outPane->appendLine( tr("Found Board definition in file: ") + board );
}

#include "moc_inodebugger.cpp"
