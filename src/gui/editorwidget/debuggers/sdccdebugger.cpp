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

#include <QFileInfo>
#include <QDebug>

#include "sdccdebugger.h"
#include "mcuinterface.h"
#include "codeeditor.h"
#include "utils.h"

#include "gputilsdebug.h"

SdccDebugger::SdccDebugger( CodeEditor* parent, OutPanelText* outPane )
            : cDebugger( parent, outPane )
{
    m_family = "pic14";
}
SdccDebugger::~SdccDebugger(){}

int SdccDebugger::compile( bool debug )
{
    int error = Compiler::compile( debug );
    if( error == 0 && !m_family.startsWith("pic") )
    {
        if( !QFileInfo::exists( m_firmware ) )
        {
            QString ihx = m_buildPath+m_fileName+".ihx";
            if( QFileInfo::exists( ihx ) )   // Convert .ihx to .hex
            {
                QString packihx = "packihx";
            #ifndef Q_OS_UNIX
                packihx += ".exe";
            #endif
                m_compProcess.setWorkingDirectory( m_buildPath );
                m_compProcess.start( packihx+" "+m_fileName+".ihx" );
                m_compProcess.waitForFinished(-1);

                QFile file( m_buildPath+m_fileName+".hex" );
                if( file.open(QFile::WriteOnly | QFile::Text) )
                {
                    QTextStream out(&file);
                    out << m_compProcess.readAllStandardOutput();
                    file.close();
    }   }   }   }
    return error;
}

void SdccDebugger::preProcess()
{
    cDebugger::preProcess();
}

bool SdccDebugger::postProcess()
{
    m_flashToSource.clear();
    m_sourceToFlash.clear();
    m_lastLine = 0;

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
