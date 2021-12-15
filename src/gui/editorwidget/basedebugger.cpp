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
    QStringList lines = fileToStringList( m_file, "BaseDebugger::preProcess" );
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
    QStringList srcLines = fileToStringList( srcFile, "BaseDebugger::postProcess" );
    QStringList lstLines = fileToStringList( lstFile, "BaseDebugger::postProcess" );

    QString lstLine;
    int lstLineNumber = 0;
    int srcLineNumber = 0;
    int lastListLine = lstLines.size();

    if( m_langLevel ) // High level language
    {
        QString file = m_fileName+m_fileExt;
        bool found = false;
        for( QString lstLine : lstLines )
        {
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
            if( found )
            {
                if( m_lstType & 1 ) lstLine = lstLine.split(":").last();
                QStringList words = lstLine.split(" ");
                words.removeAll("");

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
                    setLineToFlash( srcLineNumber, m_codeStart+address );
                    found = false;
                    continue;
    }   }   }   }
    else{             // asm
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
                srcLine = srcLine.split("//").first();
                if( line.contains( srcLine ) ) break;          // Line found
            }
            if( lstLineNumber >= lastListLine ) lstLineNumber = 0;
            else{
                if( m_lstType & 1 ) lstLine = lstLine.split(":").last();
                QStringList words = lstLine.split(" ");
                words.removeAll("");
                lstLine = words.at( (m_lstType & 2)>>1 );

                bool ok = false;
                int address = lstLine.toInt( &ok, 16 );
                if( ok ) setLineToFlash( srcLineNumber, m_codeStart+address );
    }   }   }
    return true;
}

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
    QStringList wordList = line.replace(":", " ").replace("=", " ").split(" ");

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

void BaseDebugger::setLineToFlash( int line, int addr )
{
    if( !m_sourceToFlash.contains( line ) )
    {
        //qDebug() << " line:" << line << "addr:" << addr;
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
