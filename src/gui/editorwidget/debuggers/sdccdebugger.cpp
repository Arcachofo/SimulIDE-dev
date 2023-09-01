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
            QString packihx = "packihx";
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
                if ( words.isEmpty() ) continue;
                else if ( words.contains( ";" ) ) continue;     // comment
                else if ( words.last().endsWith( ":" ) ) continue; // label;
                else {  // don't have []
                    bool hasBrackets = false;
                    foreach (const QString& word, words) {
                        if ( word.startsWith( '[' ) && word.endsWith( ']' ) ) {
                            hasBrackets = true;
                            break;
                        }
                    }
                    if ( !hasBrackets ) continue;
                }
                found = false;

                bool ok = false;
                lstLine = words[0];
                int address = lstLine.toInt( &ok, 16 );
                if( ok )
                {
                    qDebug("%d %x", srcLineNumber, m_codeStart+address);
                    setLineToFlash( {srcFile, srcLineNumber}, m_codeStart+address );
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
