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

#include <QFileInfo>

#include "basedebugger.h"
#include "mcuinterface.h"
#include "editorwindow.h"
#include "compilerprop.h"
#include "simulator.h"
#include "mcubase.h"
#include "utils.h"

BaseDebugger::BaseDebugger( CodeEditor* parent, OutPanelText* outPane )
            : Compiler( parent, outPane )
{
    m_compName = "None";
    m_processorType = 0;
    m_langLevel = 0;
    m_lstType = 0;
    m_stepOver = false;

    m_appPath = QCoreApplication::applicationDirPath();
}
BaseDebugger::~BaseDebugger( )
{
    if( McuInterface::self() ) McuInterface::self()->getRamTable()->remDebugger( this );
}

bool BaseDebugger::upload()
{
    if( !QFileInfo::exists( m_firmware) )
    {
        m_outPane->appendLine( "\n"+tr("Error: Hex file doesn't exist:")+"\n"+m_firmware );
        return false;
    }
    if( !McuBase::self() )
    {
        m_outPane->appendLine( "\n"+tr("Error: No Mcu in Simulator... ") );
        return false;
    }
    bool ok = McuBase::self()->load( m_firmware );
    if( ok ) m_outPane->appendText( "\n"+tr("FirmWare Uploaded to ") );
    else     m_outPane->appendText( "\n"+tr("Error uploading firmware to ") );
    m_outPane->appendLine( McuBase::self()->device() );
    m_outPane->appendLine( m_firmware+"\n" );

    if( ok )
    {
        McuInterface::self()->setDebugger( this );
        ok = postProcess();
    }
    return ok;
}

void BaseDebugger::preProcess()
{
    QStringList lines = fileToStringList( m_file, "cDebugger::preProcess" );
    getInfoInFile( lines.first() );
    m_codeStart = 0;
}

bool BaseDebugger::postProcess()
{
    m_lastLine = 0;
    m_flashToSource.clear();
    m_sourceToFlash.clear();

    QString lstFile = m_buildPath+m_fileName+".lst";
    if( !QFileInfo::exists( lstFile ) )
    {
        m_outPane->appendLine( "\n"+tr("Warning: lst file doesn't exist:")+"\n"+lstFile );
        return false;
    }
    QString srcFile = m_fileDir + m_fileName + m_fileExt;
    QStringList srcLines = fileToStringList( srcFile, "AsmDebugger::postProcess" );
    QStringList lstLines = fileToStringList( lstFile, "AsmDebugger::postProcess" );

    QString lstLine;
    int lstLineNumber = 0;
    int srcLineNumber = 0;
    int lastListLine = lstLines.size();

    for( QString srcLine : srcLines )
    {
        srcLineNumber++;
        srcLine = srcLine.replace("\t", " ").remove(" ");
        if( isNoValid( srcLine ) ) continue;

        while( true )
        {
            if( ++lstLineNumber >= lastListLine ) break;      // End of lst file
            lstLine = lstLines.at( lstLineNumber-1 );
            lstLine = lstLine.replace("\t", " ");
            if( isNoValid( lstLine ) ) continue;

            QString line = lstLine;
            line = line.remove(" ");
            if( line.contains( srcLine ) )
            {
                if( m_langLevel )
                { if( line.contains( m_fileName+m_fileExt ) ) break; }// Line found
                else break;          // Line found
        }   }
        if( lstLineNumber >= lastListLine ) lstLineNumber = 0;
        else{
            if( m_langLevel )
            {
                lstLineNumber++;
                lstLine = lstLines.at( lstLineNumber-1 );
                lstLine = lstLine.replace("\t", " ");
            }
            if( m_lstType ) lstLine = lstLine.split(":").last();
            QStringList words = lstLine.split(" ");
            words.removeAll("");
            lstLine = words.first();

            bool ok = false;
            int address = lstLine.toInt( &ok, 16 );
            if( ok ) setLineToFlash( srcLineNumber, m_codeStart+address );
    }   }
    return true;
}

void BaseDebugger::getInfoInFile( QString line )
{
    line = line.toLower();
    QStringList wordList = line.split(" ");

    while( wordList.size() > 2 )
    {
        QString word = wordList.takeFirst();
        if( word == "device" || word == "board" || word == "family" )
        {
            if( word.contains("=") )
            {
                QString second = word;
                second = second.split("=").last();
                if( !second.isEmpty() ) wordList.prepend( second );
            }
            else if( wordList.takeFirst() != "=" ) continue;
            if( word == "device" )
            {
                m_device = wordList.takeFirst();
                if( m_compDialog ) m_compDialog->setDevice( m_device );
            }
            else if( word == "board" )  m_board  = wordList.takeFirst();
            else if( word == "family" ) m_family = wordList.takeFirst();
}   }   }

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

void BaseDebugger::setLineToFlash( int line, int addr )
{
    if( !m_sourceToFlash.contains( line ) )
    {
        if( line > m_lastLine ) m_lastLine = line;
        m_flashToSource[ addr ] = line;
        m_sourceToFlash[ line ] = addr;
    }
}

int BaseDebugger::getValidLine( int line )
{
    while( !m_sourceToFlash.contains(line) && line<=m_lastLine ) line++;
    return line;
}

QString BaseDebugger::getVarType( QString var )
{
    return m_varList.value( var );
}

#include "moc_basedebugger.cpp"
