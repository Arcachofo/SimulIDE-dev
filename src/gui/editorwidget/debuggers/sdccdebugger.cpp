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
    }
    else //if( m_family == "msc51" )
    {
        bool ok = findCSEG();
        if( !ok ) return false;
        return BaseDebugger::postProcess();
}   }

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
