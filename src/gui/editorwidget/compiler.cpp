/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>
#include <QFileDialog>
#include <QSettings>
#include <QDir>

#include "compiler.h"
#include "simulator.h"
#include "editorwindow.h"
#include "outpaneltext.h"
#include "compilerprop.h"
#include "mainwindow.h"
#include "utils.h"

Compiler::Compiler( CodeEditor* editor, OutPanelText* outPane )
        : QObject( editor )
        , m_compProcess( this )
{
    m_editor = editor;
    m_outPane = outPane;

    m_compDialog = NULL;

    m_file     = editor->getFilePath();
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
    m_type.clear();
    m_useDevice = false;
}

QString Compiler::replaceData( QString str )
{
    QString filePath = addQuotes( m_file );
    QString inclPath = addQuotes( m_inclPath );
    //QString buildPath = addQuotes( m_buildPath );

    str = str.replace( "$filePath", filePath )
             .replace( "$fileDir" , m_fileDir )
             .replace( "$fileName", m_fileName )
             .replace( "$fileExt" , m_fileExt )
             .replace( "$inclPath", inclPath )
             .replace( "$buildPath", m_buildPath );
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

    m_uploadHex = compiler.attribute( "uploadhex" ) != "false";
    QString inclPath = "";

    m_compName = compiler.attribute( "name" );
    m_type     = compiler.attribute( "type" );
    if( compiler.hasAttribute("syntax") ) m_editor->setSyntaxFile( compiler.attribute( "syntax" )+".syntax" );
    if( compiler.hasAttribute("buildPath") )
    {
        QString path = replaceData( compiler.attribute( "buildPath" ) );
        QDir buildDir= QFileInfo( m_file ).absoluteDir();
        if( !buildDir.cd( path ) )
        {
            buildDir.mkpath( m_fileDir+"/"+path); // Create build Dir
            buildDir.cd( path );
        }
        m_buildPath = buildDir.absolutePath()+"/";
    }
    if( compiler.hasAttribute("inclPath") ) inclPath = compiler.attribute( "inclPath" );
    if( !inclPath.isEmpty() ) m_inclPath = inclPath;

    m_useFamily = compiler.hasAttribute("useFamily")
              && (compiler.attribute("useFamily") == "true");

    m_useDevice = compiler.hasAttribute("useDevice")
              && (compiler.attribute("useDevice") == "true") ;

    QDomNode node = compiler.firstChild();
    while( !node.isNull() )
    {
        QDomElement step = node.toElement();
        if( step.tagName() == "step")
        {
            QString command   = step.attribute("command");
            QString arguments = step.attribute("arguments");
            QString argsDebug = step.attribute("argsDebug");
            if( argsDebug.isEmpty() ) argsDebug = arguments ;

            m_command.append( command );
            m_arguments.append( arguments );
            m_argsDebug.append( argsDebug );
        }
        node = node.nextSibling();
    }
    readSettings();
    //m_outPane->appendLine( "-------------------------------------------------------" );
    m_outPane->appendLine( m_compName+tr(" Compiler successfully loaded.\n") );
}

int Compiler::compile( bool debug )
{
    if     ( m_compName == "None" ) m_outPane->appendLine( tr("     No Compiler Defined") );
    else if( m_command.isEmpty() )  m_outPane->appendLine( tr("     No command Defined") );

    int error = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    m_fileList.clear();
    preProcess();

    for( int i=0; i<m_command.size(); ++i )
    {
        QString command = m_toolPath + m_command.at(i);
        if( !checkCommand( command ) )
        {
            m_outPane->appendLine( "ERROR: "+command );
            toolChainNotFound();
            error = -1;
            break;
        }
        command = addQuotes( command );

        QString arguments = debug ? m_argsDebug.at(i) : m_arguments.at(i);
        arguments = replaceData( arguments );

        if( m_useFamily && arguments.contains("$family") )
        {
            if( m_family.isEmpty() )
            {
                m_outPane->appendLine( tr("     Error: Family not defined") );
                error = -1;
                break;
            }
            else arguments = arguments.replace( "$family", m_family );
        }
        if( m_useDevice && arguments.contains("$device") )
        {
            if( m_device.isEmpty() )
            {
                m_outPane->appendLine( tr("     Error: Device not defined") );
                error = -1;
                break;
            }
            else arguments = arguments.replace( "$device", m_device );
        }
        error = runBuildStep( command + arguments );
        if( error > 0 ) break;
    }
    if( error == 0 ) compiled( m_buildPath+m_fileName+".hex" );

    QApplication::restoreOverrideCursor();
    return error;
}

int Compiler::runBuildStep( QString fullCommand )
{
    m_outPane->appendLine( "Executing:\n"+fullCommand+"\n" );
    m_compProcess.setWorkingDirectory( m_fileDir );
    m_compProcess.start( fullCommand  );
    m_compProcess.waitForFinished(-1);

    return getErrors();
}

void Compiler::compiled( QString firmware )
{
    //m_fileList.clear();
    //m_fileList.append( m_file ); //( m_fileName+m_fileExt );
    if( m_fileExt == ".hex" ) m_uploadHex = true;
    if( m_uploadHex ) m_firmware = firmware;
    else              m_firmware = "";
}

int Compiler::getErrors()
{
    int error = 0;

    QString p_stdout = m_compProcess.readAllStandardOutput();
    if( !p_stdout.isEmpty() ) error = getErrorLine( p_stdout );
    if( error ) return error;

    QString p_stderr = m_compProcess.readAllStandardError();
    if( !p_stderr.isEmpty() ) error = getErrorLine( p_stderr );
    return error;
}

int Compiler::getErrorLine( QString txt )
{
    m_outPane->appendLine( txt );

    int error = 0;
    for( QString line : txt.split("\n") )
    {
        if( !line.contains( m_fileName+m_fileExt ) ) continue;
        line = line.split( m_fileName+m_fileExt ).last();

        int errorLine = getFirstNumber( line );
        if( errorLine != 0 )
        {
            line = line.toLower();             // Make it case insensitive
            if( line.contains("error")   )
            {
                if( error == 0 ) error = errorLine;
                m_editor->addError( errorLine );
            }
            else if( line.contains("warning") ) m_editor->addWarning( errorLine );
        }
    }
    return error;
}

int Compiler::getFirstNumber( QString txt )
{
    int number = 0;
    QRegularExpression rx("[0-9]+");
    QRegularExpressionMatch match = rx.match( txt );
    if ( match.hasMatch()  ) number = match.captured(0).toInt();
    return number;
}

QString Compiler::getPath( QString msg )
{
    QString path = QFileDialog::getExistingDirectory( NULL
                         , msg
                         , toolPath()
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
    m_outPane->appendLine( "     : "+tr("Executable not found") );
    m_outPane->appendLine( "     : "+tr("Check that Tool Path is correct")+"\n" );
}

void Compiler::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();

    QString prop = m_compName+"_toolPath";
    if( settings->contains( prop ) ) setToolPath( settings->value( prop ).toString() );
    prop = m_compName+"_inclPath";
    if( settings->contains( prop ) ) m_inclPath = settings->value( prop ).toString();
}

void Compiler::compProps()
{
    if( !m_compDialog )
    {
        m_compDialog = new CompilerProp( m_editor );
        m_compDialog->setCompiler( this );
    }
    m_compDialog->show();
}

bool Compiler::checkCommand( QString executable )
{
    if( QFile::exists( executable ) ) return true;

    QProcess check;
    check.start( executable  );
    bool started = check.waitForStarted();
    if( started && !check.waitForFinished(1000) ) check.kill();
    check.readAllStandardError();
    check.readAllStandardOutput();

    return started;
}

#include "moc_compiler.cpp"
