/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileInfo>

#include "basedebugger.h"
#include "editorwindow.h"
#include "compilerprop.h"
#include "simulator.h"
#include "cpubase.h"
#include "mcu.h"
#include "utils.h"

BaseDebugger::BaseDebugger( CodeEditor* parent, OutPanelText* outPane )
            : Compiler( parent, outPane )
{
    m_compName = "None";
    m_langLevel = 0;
    m_lstType = 0;

    m_appPath = QCoreApplication::applicationDirPath();
}
BaseDebugger::~BaseDebugger( )
{
    if( eMcu::self() ) eMcu::self()->getRamTable()->remDebugger( this );
}

bool BaseDebugger::upload()
{
    if( !m_firmware.isEmpty() && !QFileInfo::exists( m_firmware ) )
    {
        m_outPane->appendLine( "\n"+tr("Error: Hex file doesn't exist:")+"\n"+m_firmware );
        return false;
    }
    if( !Mcu::self() )
    {
        m_outPane->appendLine( "\n"+tr("Error: No Mcu in Simulator... ") );
        return false;
    }
    bool ok = true;
    if( !m_firmware.isEmpty() )
    {
        ok = Mcu::self()->load( m_firmware );
        if( ok ) m_outPane->appendText( "\n"+tr("FirmWare Uploaded to ") );
        else     m_outPane->appendText( "\n"+tr("Error uploading firmware to ") );
        m_outPane->appendLine( Mcu::self()->device() );
        m_outPane->appendLine( m_firmware+"\n" );
    }
    if( ok ){
        m_debugStep = false;
        m_stepOver = false;
        m_running = false;
        eMcu::self()->setDebugger( this );
        if( m_fileExt != ".hex" ) ok = postProcess();
    }
    return ok;
}

void BaseDebugger::preProcess()
{
    QStringList lines = fileToStringList( m_file, "BaseDebugger::preProcess" );
    getInfoInFile( lines.first() );
    m_codeStart = 0;
}

bool BaseDebugger::postProcess()
{
    //m_lastLine = 0;
    m_flashToSource.clear();
    //m_sourceToFlash.clear();

    QString lstFile = m_buildPath+m_fileName+".lst";
    if( !QFileInfo::exists( lstFile ) )
    {
        m_outPane->appendLine( "\n"+tr("Warning: lst file doesn't exist:")+"\n"+lstFile );
        return false;
    }
    m_outPane->appendText( "\nMapping Flash to Source... " );
    QString srcFile = m_fileDir + m_fileName + m_fileExt;
    QStringList srcLines = fileToStringList( srcFile, "BaseDebugger::postProcess" );
    QStringList lstLines = fileToStringList( lstFile, "BaseDebugger::postProcess" );

    QString lstLine;
    int lstLineNumber = 0;
    int srcLineNumber = 0;
    int listFileSize = lstLines.size();
    int lastListLine = 0;

    if( m_langLevel ) // High level language
    {
        QString file = m_fileName+m_fileExt;
        bool found = false;
        for( QString lstLine : lstLines )
        {
            lstLineNumber++;
            if( found )
            {
                found = false;
                if( m_lstType & 1 ) lstLine = lstLine.split(":").last();
                QStringList words = lstLine.split(" ");
                words.removeAll("");
                if( words.size() < 4 ) continue;

                bool ok = false;
                int addrIndex = (m_lstType & 2)>>1;
                if( addrIndex )  // check if line valid: first item should be a number
                {
                    words.at( 0 ).toInt( &ok, 16 );
                    if( !ok ) continue;
                }
                lstLine = words.at( addrIndex );
                int address = lstLine.toInt( &ok, 16 );
                if( ok )
                {
                    setLineToFlash( {srcFile, srcLineNumber}, m_codeStart+address );
                    continue;
            }   }
            if( lstLine.contains( file ) )
            {
                QString str = lstLine.split( file ).takeLast();
                QStringList words = str.remove(":").split(" ");
                words.removeAll("");
                if( words.isEmpty() ) continue;
                str = words.first();
                bool ok = false;
                srcLineNumber = str.toInt( &ok );
                if( ok ) found = true;
            }
        }
    }
    else{                        // asm
        QString funcName;
        for( QString srcLine : srcLines ) // Get Functions
        {
            if( isNoValid( srcLine ) ) continue;
            srcLine = srcLine.split(";").first();
            if( !srcLine.toUpper().contains("CALL") ) continue;
            srcLine = srcLine.replace("\t", " ");
            QStringList l = srcLine.split(" ");
            l.removeAll("");
            funcName = l.last();
            if( !funcName.isEmpty() ) m_functions[funcName.toUpper()] = -1;
        }

        for( QString srcLine : srcLines )
        {
            srcLineNumber++;
            srcLine = srcLine.replace("\t", " ").remove(" ");
            if( isNoValid( srcLine ) ) continue;
            srcLine = srcLine.split(";").first();
            lastListLine = lstLineNumber;

            while( true )
            {
                if( ++lstLineNumber >= listFileSize ) break;   // End of lst file
                lstLine = lstLines.at( lstLineNumber-1 );
                lstLine = lstLine.replace("\t", " ");
                if( isNoValid( lstLine ) ) continue;

                lstLine = lstLine.split(";").first();
                QString line = lstLine;
                line = line.remove(" ");
                if( line.contains( srcLine ) ) break;          // Line found
            }
            if( lstLineNumber >= listFileSize )
            {
                lstLineNumber = lastListLine; /// lstLineNumber = 0;
                continue;
            }
            if( m_lstType & 1 )
            {
                QStringList l = lstLine.split(":");
                if( l.size() > 1 ) lstLine = lstLine.split(":").at( 1 );
                else continue;
            }
            QStringList words = lstLine.split(" ");
            words.removeAll("");

            if( words.size() < 3 ){
                if( srcLine.contains( ":" ) )                 // Find Subroutines
                {
                    funcName = srcLine.left( srcLine.indexOf(":") ).toUpper();
                    if( !m_functions.contains( funcName ) ) funcName = "";
                }
                continue;
            }
            int index = (m_lstType & 2)>>1 ;
            bool ok = false;
            words.at( index+1 ).toInt( &ok, 16 ); // Avoid things like "8: E = %10000000" (vasm)
            if( !ok ) continue;

            int address = m_codeStart+words.at( index ).toInt( &ok, 16 );
            if( ok )
            {
                setLineToFlash( {srcFile, srcLineNumber}, address );
                if( !funcName.isEmpty() )                  // Subroutine starting here
                {
                    m_functions[funcName] = address;
                    funcName = "";
                }
            }
        }
    }
    m_outPane->appendLine( QString::number( m_flashToSource.size() )+" lines mapped" );
    return true;
}

void BaseDebugger::run()
{
    m_running = true;
    stepFromLine();
}
void BaseDebugger::pause()
{
    m_running = false;
    m_debugStep = false;
    EditorWindow::self()->pauseAt( m_prevLine );
}

bool BaseDebugger::stepFromLine( bool over )
{
    bool ok = true;
    if( m_prevLine.lineNumber == -1 ) // Jump from line 1 to flash addr = 0
    {
        if( m_flashToSource.keys().contains(0) )
        {
            codeLine_t l = m_flashToSource.value( 0 );
            m_prevLine = l;
            EditorWindow::self()->pauseAt( m_prevLine );
            ok = false;
        }
    }
    m_over = over;
    m_exitPC = 0;
    m_debugStep = true;

    return ok;
}

void BaseDebugger::stepDebug()
{
    if( !m_debugStep ) return;

    int lastPC = eMcu::self()->cpu->getPC();
    eMcu::self()->stepCpu();
    int PC = eMcu::self()->cpu->getPC();

    if( lastPC != PC )
    {
        if( m_over ){       // Step Over entry
            if( m_functions.values().contains( PC ) )
            {
                m_exitPC = eMcu::self()->cpu->RET_ADDR();
                m_over = false;
                return;
            }
        }
        if( m_exitPC )     // Step Over exit
        {
            if( PC == m_exitPC ) m_exitPC = 0;
            else return;
        }
        if( m_flashToSource.contains( PC ) )
        {
            codeLine_t line = m_flashToSource.value( PC );
            if( line != m_prevLine )
            {
                m_prevLine = line;
                EditorWindow::self()->lineReached( line );
}   }   }   }

void BaseDebugger::getInfoInFile( QString line )
{
    QString device = getValue( line, "device" );
    if( !device.isEmpty() )
    {
        m_device = device;
        if( m_compDialog ) m_compDialog->setDevice( m_device );
        m_outPane->appendLine( tr("Found Device definition in file: ") + device );
    }
    QString board = getValue( line, "board" );
    if( !board.isEmpty() ) setBoardName( board );

    QString family = getValue( line, "family" );
    if( !family.isEmpty() )
    {
        m_family = family;
        if( m_compDialog ) m_compDialog->setFamily( m_family );
        m_outPane->appendLine( tr("Found Family definition in file: ") + family );
    }
}

QString BaseDebugger::getValue( QString line, QString key ) // Static
{
    QString lineL = line.toLower();
    if( !lineL.contains( key.toLower() ) ) return "";

    QString value = "";
    QStringList wordList = line.replace("=", " ").split(" ");

    while( wordList.size() > 1 )
    {
        QString word = wordList.takeFirst();
        if( word.isEmpty() ) continue;
        if( word.toLower().contains( key ) )
        {
            while( !wordList.isEmpty() && value.isEmpty() )
                value = wordList.takeFirst();
            value = value.remove(",").remove(".").remove(";");
    }   }
    return value;
}

bool BaseDebugger::isNoValid( QString line )
{
    return (  line.isEmpty()
           || line.startsWith("void")
           || line.startsWith("{")
           || line.startsWith("}")
           || line.startsWith("//")
           || line.startsWith("/*")
           || line.startsWith(";")
           || line.startsWith("#")
           || line.startsWith(".") );
}

void BaseDebugger::setLineToFlash( codeLine_t line, int addr )
{
    //int lineNumber = line.lineNumber;
    if( !m_flashToSource.contains( addr ) )
    {
        //qDebug() << " line:" << line << "addr:" << addr;
        //if( lineNumber > m_lastLine ) m_lastLine = lineNumber;
        m_flashToSource[ addr ] = line;
        ///qDebug() <<addr<<line.lineNumber<<line.file;
        //m_sourceToFlash[ lineNumber ] = addr;
    }
}

int BaseDebugger::getValidLine( codeLine_t line )
{
    int lineNumber = line.lineNumber;
    int lastLine = 0;
    QList<int> lineList;
    for( codeLine_t cd : m_flashToSource.values() ){
        if( line.file == cd.file )
        {
            if( cd.lineNumber > lastLine ) lastLine = cd.lineNumber;
            lineList.append( cd.lineNumber );
        }
    }
    while( !lineList.contains( lineNumber ) && lineNumber<=lastLine ) lineNumber++;
    if( lineNumber == lastLine && !lineList.contains( lineNumber ) ) return -1; // No valid line found
    return lineNumber;
}

bool BaseDebugger::isMappedLine( codeLine_t line )
{
    return m_flashToSource.values().contains(line);
}

QString BaseDebugger::getVarType( QString var )
{
    return m_varTypes.value( var );
}

#include "moc_basedebugger.cpp"
