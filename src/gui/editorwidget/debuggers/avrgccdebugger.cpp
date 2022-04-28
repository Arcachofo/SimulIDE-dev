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

#include <QDebug>
#include <QFileInfo>

#include "avrgccdebugger.h"
#include "e_mcu.h"
#include "outpaneltext.h"
#include "codeeditor.h"
#include "utils.h"

AvrGccDebugger::AvrGccDebugger( CodeEditor* parent, OutPanelText* outPane )
              : cDebugger( parent, outPane )
{
    m_addrBytes = 1; // Default for avr-gcc
}
AvrGccDebugger::~AvrGccDebugger(){}

bool AvrGccDebugger::postProcess()
{
    bool ok = getVariables();
    if( !ok ) return false;

    m_flashToSource.clear();
    m_sourceToFlash.clear();
    return mapFlashToSource();
}

bool AvrGccDebugger::getVariables()
{
    QString elfPath = m_buildPath+m_fileName+".elf";
    if( !QFileInfo::exists( elfPath ) )
    {
        m_outPane->appendLine( "\n"+QObject::tr("Warning: elf file doesn't exist:")+"\n"+elfPath );
        return false;
    }
    QString objdump = m_toolPath+"avr/bin/avr-objdump";

#ifndef Q_OS_UNIX
    objdump += ".exe";
#endif

    if( !checkCommand( objdump ) )
    {
        objdump = m_toolPath+"avr-objdump";
    #ifndef Q_OS_UNIX
        objdump += ".exe";
    #endif
        if( !checkCommand( objdump ) )
            outPane()->appendLine( "\nWarning: avr-objdump executable not detected:\n"+objdump );
    }
    outPane()->appendText( "\nSearching for variables... " );
    objdump = addQuotes( objdump );
    elfPath = addQuotes( elfPath );

    QProcess getBss( NULL );      // Get var addresses from .bss section
    QString command  = objdump+" -t -j.bss "+elfPath;
    getBss.start( command );
    getBss.waitForFinished(-1);

    QString  p_stdout = getBss.readAllStandardOutput();
    QStringList varNames = m_varTypes.keys();
    QStringList varList;
    m_subs.clear();

    for( QString line : p_stdout.split("\n") )
    {
        if( line.isEmpty() ) continue;
        QStringList words = line.split(" ");
        if( words.size() < 9 ) continue;
        if( words.at(6) != "O" ) continue;

        QString addr   = words.at(0);
        bool ok = false;
        int address = addr.toInt( &ok, 16 );
        if( !ok ) continue;

        QString symbol = words.at(8);
        QString type;

        if( varNames.contains( symbol ) ) type = m_varTypes.value( symbol );
        else{
            QString size = words.at(7);
            size = size.split("\t").last();
            type = "u"+QString::number( size.toInt()*8 );
        }
        address -= 0x800000;          // 0x800000 offset

        eMcu::self()->getRamTable()->addVariable( symbol, address, type );
        varList.append( symbol );
        //qDebug() << "AvrGccDebugger::getAvrGccData  variable "<<type<<symbol<<address;
    }
    eMcu::self()->getRamTable()->setVariables( varList );
    outPane()->appendLine( QString::number( varList.size() )+" variables found" );
    return true;
}

bool AvrGccDebugger::mapFlashToSource()
{
    QString elfPath = m_buildPath+m_fileName+".elf";
    if( !QFileInfo::exists( elfPath ) )
    {
        m_outPane->appendLine( "\n"+QObject::tr("Warning: elf file doesn't exist:")+"\n"+elfPath );
        return false;
    }
    QString avrSize = m_toolPath+"avr/bin/avr-size";
    QString addr2li = m_toolPath+"avr/bin/avr-addr2line";

#ifndef Q_OS_UNIX
    avrSize += ".exe";
    addr2li += ".exe";
#endif
    if( !checkCommand( avrSize ) )
    {
        avrSize = m_toolPath+"avr-size";
    #ifndef Q_OS_UNIX
        avrSize += ".exe";
    #endif
        if( !checkCommand( avrSize ) )
            outPane()->appendLine( "\nWarning: avr-size executable not detected:\n"+avrSize );
    }
    if( !checkCommand( addr2li ) )
    {
        addr2li = m_toolPath+"avr-addr2line";
    #ifndef Q_OS_UNIX
        addr2li += ".exe";
    #endif
        if( !checkCommand( addr2li ) )
            outPane()->appendLine( "\nWarning: avr-addr2line executable not detected:\n"+addr2li );
    }
    outPane()->appendText( "\nMapping Flash to Source... " );
    avrSize = addQuotes( avrSize );
    addr2li = addQuotes( addr2li );
    elfPath = addQuotes( elfPath );

    QProcess getSize( this );  // Get Firmware size
    getSize.start( avrSize + " " + elfPath );
    getSize.waitForFinished();
    QString lines = getSize.readAllStandardOutput();
    getSize.close();
    bool ok = false;
    int flashSize;
    if( !lines.isEmpty() )
    {
        QString size = lines.split("\n").at(1);
        size = size.split("\t").takeFirst().remove(" ");
        flashSize = size.toInt( &ok );
    }
    if( !ok || flashSize == 0 ) flashSize = 35000;

    QProcess flashToLine( this );
    flashToLine.start( addr2li + " -e " + elfPath );
    ok = flashToLine.waitForStarted( 1000 );
    if( ok )
    {
        for( int flashAddr=0; flashAddr<flashSize; ++flashAddr ) // Map Flash Address to Source Line
        {
            QString addr = val2hex( flashAddr )+"\n";
            flashToLine.write( addr.toUtf8() );
        }
        flashToLine.closeWriteChannel();
        flashToLine.waitForFinished();

        m_lastLine = 0;
        for( int flashAddr=0; flashAddr<flashSize; ++flashAddr ) // Map Flash Address to Source Line
        {
            QString p_stdout = flashToLine.readLine();
            if( p_stdout.isEmpty() ) continue;
            if( p_stdout.startsWith("?") ) continue;

            //m_outPane->appendLine(p_stdout);
            int idx = p_stdout.lastIndexOf( ":" );
            if( idx == -1 ) continue;

            QString fileName = QFileInfo( p_stdout.left( idx ) ).fileName();
            if( m_fileList.contains( fileName ) )
            {
                bool ok = false;
                int line = p_stdout.mid( idx+1 ).toInt( &ok );
                if( !ok ) continue;
                int addr = flashAddr*m_addrBytes/2;
                setLineToFlash( line, addr );
        }   }
        flashToLine.close();
    }
    ok = !m_flashToSource.isEmpty();
    outPane()->appendLine( QString::number( m_flashToSource.size() )+" lines mapped" );
    return ok;
}

