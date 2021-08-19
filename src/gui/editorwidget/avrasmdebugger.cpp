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

#include "avrasmdebugger.h"
#include "baseprocessor.h"
#include "mainwindow.h"
#include "utils.h"
#include "simuapi_apppath.h"

static const char* AvrAsmDebugger_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Avra Inc Path")
};

AvrAsmDebugger::AvrAsmDebugger( CodeEditor* parent, OutPanelText* outPane, QString filePath )
              : BaseDebugger( parent, outPane, filePath )
{
    Q_UNUSED( AvrAsmDebugger_properties );
    
    setObjectName( "AVR asm Compiler/Debugger" );
    
    m_compilerPath = "";
    m_avraIncPath  = "";
    m_compSetting = "avra_Path";
    
    readSettings();
    
    QSettings* settings = MainWindow::self()->settings();
    
    if( settings->contains("avra_Inc_Path") )
        m_avraIncPath = settings->value("avra_Inc_Path").toString();
    
    if( m_avraIncPath == "" )
        m_avraIncPath = SIMUAPI_AppPath::self()->availableDataDirPath("codeeditor/tools/avra");

    m_incDir = m_avraIncPath;

    m_typesList["byte"]    = "uint8";
}
AvrAsmDebugger::~AvrAsmDebugger() {}

void AvrAsmDebugger::mapFlashToSource()
{
    m_flashToSource.clear();
    m_sourceToFlash.clear();

    QString asmFileName = m_fileDir + m_fileName + ".asm";
    QString lstFileName = m_fileDir + m_fileName + ".lst";

    QStringList asmLines = fileToStringList( asmFileName, "AvrAsmDebugger::mapLstToAsm" );
    QStringList lstLines = fileToStringList( lstFileName, "AvrAsmDebugger::mapLstToAsm" );
    
    m_lastLine = 0;

    QString asmLine;
    int asmLineNumber = 0;
    int lastAsmLine = asmLines.size();

    for( QString lstLine : lstLines )
    {
        if( !lstLine.startsWith( "C:") ) continue;            // avra code lines start with C:

        lstLine = lstLine.toUpper().remove(0,2).replace("\t", " ").remove((" "));

        while( true )
        {
            if( ++asmLineNumber >= lastAsmLine ) break;                   // End of asm file
            asmLine = asmLines.at( asmLineNumber ).toUpper();
            asmLine = asmLine.replace("\t", " ").remove(" ");
            if( asmLine.isEmpty() ) continue;
            if( asmLine.startsWith(";")) continue;
            if( asmLine.startsWith("#")) continue;
            if( asmLine.startsWith(".")) continue;

            if( lstLine.contains(asmLine) ) break;                            // Line found
        }
        QString numberText = lstLine.left( 6 );    // first 6 digits in lst file is address
        bool ok = false;
        int address = numberText.toInt( &ok, 16 );         /// adress*2: instruc = 2 bytes
        if( ok )
        {
            m_flashToSource[address] = asmLineNumber;
            if( asmLineNumber > m_lastLine ) m_lastLine = asmLineNumber;
    }   }
    QHashIterator<int, int> i(m_flashToSource);
    while( i.hasNext() )
    {
        i.next();
        int address       = i.key();
        int asmLineNumber = i.value();
        m_sourceToFlash[asmLineNumber] = address;
}   }

void AvrAsmDebugger::setAvraIncPath( QString path )
{
    m_avraIncPath = path;
    MainWindow::self()->settings()->setValue( "avra_Inc_Path", m_avraIncPath );
}

#include "moc_avrasmdebugger.cpp"
