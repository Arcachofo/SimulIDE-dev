/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileInfo>
#include <QDebug>
#include <QDateTime>

#include "sdccdebugger.h"
#include "codeeditor.h"
#include "utils.h"
#include "outpaneltext.h"

#include "gputilsdebug.h"

SdccDebugger::SdccDebugger( CodeEditor* parent, OutPanelText* outPane )
            : cDebugger( parent, outPane )
{
    //m_family = "pic14";
}
SdccDebugger::~SdccDebugger(){}

int SdccDebugger::compile( bool debug )
{
    int error = Compiler::compile( debug );
    if( error == 0 && !m_family.startsWith("pic") )
    {
        QFileInfo ihxInfo(m_buildPath+m_fileName+".ihx");
        QFileInfo hexInfo(m_buildPath+m_fileName+".hex");

        if( !hexInfo.exists() // hex file not exists
         || ( ihxInfo.exists() && (hexInfo.lastModified() < ihxInfo.lastModified()))) // ihx file is newer
        {
            QString packihx = m_toolPath+"packihx";
        #ifndef Q_OS_UNIX
            packihx += ".exe";
        #endif
            m_compProcess.setWorkingDirectory( m_buildPath );
            m_compProcess.start( packihx+" "+m_fileName+".ihx" );
            m_compProcess.waitForFinished(-1);

            QFile file( m_buildPath+m_fileName+".hex" );
            if( file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) )
            {
                QTextStream out(&file);
                out << m_compProcess.readAllStandardOutput();
                file.close();
    }   }   }
    return error;
}

bool SdccDebugger::postProcess()
{
    m_flashToSource.clear();
    //m_sourceToFlash.clear();
    //m_lastLine = 0;

    if( m_family.startsWith("pic") )
    {
        bool ok = GputilsDebug::getVariables( this );
        if( !ok ) return false;
        return GputilsDebug::mapFlashToSource( this );
    } else {
        bool ok = findCSEG();
        if( !ok ) return false;

        m_langLevel = m_fileExt != ".asm";

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
        QStringList lstLines = fileToStringList( lstFile, "SdccDebugger::postProcess" );

        int lstLineNumber = 0;
        int srcLineNumber = 0;

        QString file = m_fileName+m_fileExt;
        bool found = false;

        if( m_langLevel ) // High level language
        {
            for( QString lstLine : lstLines )
            {
                lstLineNumber++;
                if( lstLine.contains( file ) )
                {
                    QString str = lstLine.split( file ).takeLast();
                    QStringList words = str.remove(":").split(" ");
                    words.removeAll("");
                    if( words.isEmpty() ) continue;
                    str = words.first();
                    bool ok = false;
                    srcLineNumber = str.toInt( &ok );
                    if( ok ) {
                        found = true;
                    }
                } else if( found )
                {
                    QStringList words = lstLine.split(" ");
                    words.removeAll("");
                    if( words.isEmpty() ) continue;
                    else if( words.contains( ";" ) ) continue;     // comment
                    else if( words.last().endsWith( ":" ) ) continue; // label;
                    else {  // don't have []
                        bool hasBrackets = false;
                        foreach( const QString& word, words ) {
                            if( word.startsWith( '[' ) && word.endsWith( ']' ) ) {
                                hasBrackets = true;
                                break;
                            }
                        }
                        if( !hasBrackets ) continue;
                    }
                    found = false;

                    bool ok = false;
                    lstLine = words[0];
                    int address = lstLine.toInt( &ok, 16 );
                    if( ok )
                    {
                        //qDebug("%d %x", srcLineNumber, m_codeStart+address);
                        setLineToFlash( {srcFile, srcLineNumber}, m_codeStart+address );
                    }
                }
            }
        }else // asm
        {
            int listFileSize = lstLines.size();
            int lastListLine = 0;
            QString lstLine;
            QStringList srcLines = fileToStringList( srcFile, "BaseDebugger::postProcess" );
            QString funcName;
            for( QString srcLine : srcLines ) // Get Functions
            {
                if( srcLine.startsWith(";")) continue;
                if( !srcLine.toUpper().contains("CALL") ) continue;
                srcLine = srcLine.replace("\t", " ");
                QStringList l = srcLine.split(" ");
                l.removeAll("");
                funcName = l.last();
                if( !funcName.isEmpty() ) m_functions[funcName.toUpper()] = -1;
            }

            bool areaCSEG = false;
            for( QString srcLine : srcLines )
            {
                srcLineNumber++;
                srcLine = srcLine.replace("\t", " ").remove(" ");
                if( srcLine.startsWith(";")) continue;
                if( srcLine.contains(".area") ){
                    areaCSEG = srcLine.contains("CSEG");
                    continue;
                }
                lastListLine = lstLineNumber;

                while( true )
                {
                    if( ++lstLineNumber >= listFileSize ) break;   // End of lst file
                    lstLine = lstLines.at( lstLineNumber-1 );
                    lstLine = lstLine.replace("\t", " ");

                    QString line = lstLine;
                    line = line.remove(" ");
                    if( line.contains( srcLine ) ) break;          // Line found
                }
                if( lstLineNumber >= listFileSize )
                {
                    lstLineNumber = lastListLine; /// lstLineNumber = 0;
                    continue;
                }
                QStringList words = lstLine.split(" ");
                words.removeAll("");

                if( words.size() < 5 ){
                    if( words.last().endsWith( ":" ) )                 // Find Subroutines
                    {
                        funcName = srcLine.left( srcLine.indexOf(":") ).toUpper();
                        if( !m_functions.contains( funcName ) ) funcName = "";
                    }
                    continue;
                }
                if( !lstLine.contains("[") ) continue;

                bool ok = false;
                int address = words.at( 0 ).toInt( &ok, 16 ); // Avoid things like "8: E = %10000000" (vasm)
                if( !ok ) continue;
                if( areaCSEG ) address += m_codeStart;

                m_flashToSource[ address ] = {srcFile, srcLineNumber};
                if( !funcName.isEmpty() )                  // Subroutine starting here
                {
                    m_functions[funcName] = address;
                    funcName = "";
                }
            }
        }
        m_outPane->appendLine( QString::number( m_flashToSource.size() )+" lines mapped" );
        return true;
    }
}

bool SdccDebugger::findCSEG()
{
    m_codeStart = 0;

    QString mapFileName = m_buildPath + m_fileName + ".map";
    if( !QFileInfo::exists( mapFileName ) ) return false;
    QStringList mapLines = fileToStringList( mapFileName, "SdccDebugger::findCSEG" );
    for( QString mapLine : mapLines )
    {
        if( mapLine.startsWith("CSEG") )
        {
            QStringList words = mapLine.split(" ");
            words.removeAll("");
            if( words.size() < 2 ) break;
            bool ok = false;
            int codeStart = words.at(1).toInt( &ok, 16 );
            if( ok ) m_codeStart = codeStart;
            return ok;
    }   }
    return false;
}
