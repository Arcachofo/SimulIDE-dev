/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QApplication>
#include <QSettings>

#include "inodebugger.h"
#include "outpaneltext.h"
#include "mainwindow.h"
#include "simulator.h"
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

    m_Ardboard = Uno;
    m_ArdboardList << "uno" << "megaADK" << "nano" << "diecimila" << "leonardo" << "custom";
    m_buildPath = MainWindow::self()->getFilePath("data/codeeditor/buildIno");
}
InoDebugger::~InoDebugger() {}

void InoDebugger::setToolPath( QString path )
{
    m_arduinoPath = path;
    m_toolPath = path+"hardware/tools/avr/bin/";
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
    QString builder = "arduino-builder";
#ifndef Q_OS_UNIX
    builder += ".exe";
#endif

    if( !QFile::exists( m_arduinoPath+builder) )
    {
        m_outPane->appendText( "\nArduino" );
        toolChainNotFound();
        return -1;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString filePath = m_file;

    QDir dir( m_buildPath );
    bool b = dir.cd( "build" );
    if( b ) dir.removeRecursively(); // Remove old files
    dir.mkpath( m_buildPath+"/cache");  // Create cache folder ( if doesn't exist )
    dir.mkpath( m_buildPath+"/build");  // Create build folder

    if( !QFile::exists(m_buildPath+"/build") || !QFile::exists(m_buildPath+"/cache") )
    {
        m_outPane->appendLine( "\n    ERROR: Build folders NOT found at:\n    "+m_buildPath );
        return -1;
    }
    QString command = m_arduinoPath+"arduino";

    if( m_sketchBook.isEmpty() ) // Find sketchBook
    {
    #ifndef Q_OS_UNIX
        command += "_debug.exe";
    #endif
        command = addQuotes( command );
        command += " --get-pref sketchbook.path";

        QProcess getSkBook( this );  // Get sketchBook Path
        getSkBook.start( command );
        getSkBook.waitForFinished();
        m_sketchBook = getSkBook.readAllStandardOutput();
        m_sketchBook = m_sketchBook.remove("\r").remove("\n");
        getSkBook.close();
    }
    filePath           = addQuotes( filePath );
    command            = addQuotes( m_arduinoPath+builder );
    QString hardware   = addQuotes( m_arduinoPath+"hardware" );
    QString toolsBuild = addQuotes( m_arduinoPath+"tools-builder" );
    QString toolsAvr   = addQuotes( m_arduinoPath+"hardware/tools/avr" );
    QString libraries  = addQuotes( m_arduinoPath+"libraries" );
    QString userLibrar = addQuotes( m_sketchBook+"/libraries" );
    QString cBuildPath = addQuotes( m_buildPath+"/build" );
    QString cCachePath = addQuotes( m_buildPath+"/cache" );

    QString boardSource;
    QString boardName = getBoard();
    if( boardName.isEmpty() ){
        if( m_Ardboard == Custom ) boardSource = "Custom ";
        else                       boardSource = "Arduino";
    }else                          boardSource = "In File";

    if( m_Ardboard < Custom ) boardName = "arduino:avr:"+m_ArdboardList.at( m_Ardboard );
    else                      boardName = m_customBoard;

    command += " -compile";
    command += " -hardware "+hardware;
    command += " -tools "+toolsBuild;
    command += " -tools "+toolsAvr;
    command += " -built-in-libraries "+libraries;
    command += " -libraries "+userLibrar;
    command += " -fqbn="+boardName;
    command += " -build-path "+cBuildPath;
    command += " -build-cache "+cCachePath;
    command += " "+filePath;
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
    if( !p_stderr.isEmpty() ) error = getError( p_stderr );
    else{
        m_fileList.clear();
        m_fileList.append( m_fileName+m_fileExt );
        m_firmware = m_buildPath+"/build/"+m_fileName+".ino.hex";
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
