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
//#include <QDebug>

#include "sdccdebugger.h"
#include "mcuinterface.h"
#include "codeeditor.h"
#include "utils.h"

SdccDebugger::SdccDebugger( CodeEditor* parent, OutPanelText* outPane )
              : cDebugger( parent, outPane )
{
    m_family = "pic14";
}
SdccDebugger::~SdccDebugger(){}

void SdccDebugger::getData()
{
    cDebugger::getData();

    if( !McuInterface::self() ) return;

    QString gpvc = m_toolPath+"gpvc";
    QString codPath = m_fileName+".cod";

#ifndef Q_OS_UNIX
    gpvc += ".exe";
#endif

    gpvc = addQuotes( gpvc );
    codPath = addQuotes( codPath );

    QProcess getVars( NULL );      // Get var addresses from Symbol Table
    getVars.setWorkingDirectory( m_buildPath );
    QString command  = gpvc+" -s "+codPath;
    getVars.start( command );
    getVars.waitForFinished(-1);

    QString  p_stdout = getVars.readAllStandardOutput();
    QStringList varNames = m_varList.keys();
    m_varNames.clear();
    m_subs.clear();

    for( QString line : p_stdout.split("\n") )
    {
        if( line.isEmpty() ) continue;
        line = line.remove("(").remove(")").remove("=").remove(",").replace("\t"," ");
        QStringList words = line.split(" ");
        words.removeAll("");
        if( words.size() < 5 ) continue;

        QString symbolType = words.at(4);
        QString addr   = words.at(1);
        bool ok = false;
        int address = addr.toInt( &ok, 16 );
        if( !ok ) continue;

        QString symbol = words.at(0);
        if( symbol.startsWith("_") ) symbol.remove( 0, 1 );

        if( symbolType == "address" ) continue; // Get Subs
        if( symbolType != "c_short" ) continue;
        QString type;

        if( varNames.contains( symbol ) ) type = m_varList.value( symbol );
        /*else{
            QString size = words.at(4);
            if( size.startsWith("c_") ) size.remove( 0, 2 );

            if( m_typesList.contains(size) ) type = m_typesList.value( size );
        }*/
        if( type.isEmpty() ) continue;
        McuInterface::self()->addWatchVar( symbol, address, type );
        m_varNames.append( symbol );
        //qDebug() << "SdccDebugger::getData  variable "<<type<<symbol<<address;
    }
}

void SdccDebugger::mapFlashToSource()
{
    m_flashToSource.clear();
    m_sourceToFlash.clear();

    QString gpvc = m_toolPath+"gpvc";
    QString codPath = m_fileName+".cod";

#ifndef Q_OS_UNIX
    gpvc += ".exe";
#endif

    gpvc = addQuotes( gpvc );
    codPath = addQuotes( codPath );

    QProcess flashToLine( NULL );      // Get var addresses from Symbol Table
    flashToLine.setWorkingDirectory( m_buildPath );
    QString command  = gpvc+" -l "+codPath;
    flashToLine.start( command );
    flashToLine.waitForFinished(-1);
    QString  p_stdout = flashToLine.readAllStandardOutput();

    m_lastLine = 0;
    bool readAddr = false;
    QString lineNum;

    for( QString line : p_stdout.split("\n") )
    {
        if( line.isEmpty() ) continue;
        if( readAddr )
        {
            readAddr = false;
            line = line.replace("\t"," ");
            QStringList words = line.split(" ");
            words.removeAll("");
            if( words.size() < 5 ) continue;

            bool ok = false;
            int lineN = lineNum.toInt( &ok );
            if( !ok ) continue;

            ok = false;
            int addr = words.at(2).toInt( &ok, 16 );
            if( !ok ) continue;

            if( !m_sourceToFlash.contains( lineN ) )
            {
                if( lineN > m_lastLine ) m_lastLine = lineN;
                m_flashToSource[ addr ] = lineN;
                m_sourceToFlash[ lineN ] = addr;
            }
            continue;
        }
        if( line.startsWith( ";") && line.contains(".line") )
        {
            QStringList words = line.split("\"");
            QString file = getFileName( words.at(1) );
            if( m_fileList.contains( file ) )
            {
                lineNum = words.at(0);
                lineNum = lineNum.remove(";").replace("\t"," ");
                words = lineNum.split(" ");
                words.removeAll("");
                if( words.size() < 2 ) continue;
                lineNum = words.at(1);
                readAddr = true;
}   }   }   }

