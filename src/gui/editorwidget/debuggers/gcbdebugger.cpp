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

#include "gcbdebugger.h"
#include "mcuinterface.h"
#include "outpaneltext.h"
#include "utils.h"

GcbDebugger::GcbDebugger( CodeEditor* parent, OutPanelText* outPane )
           : BaseDebugger( parent, outPane )
{
    m_stepOver = true;

    m_typesList["byte"]    = "uint8";
    m_typesList["integer"] = "int16";
    m_typesList["word"]    = "uint16";
    m_typesList["long"]    = "uint32";
    m_typesList["string"]  = "string";
}
GcbDebugger::~GcbDebugger(){}

void GcbDebugger::getSubs()
{
    m_subs.clear();
    
    QStringList lines = fileToStringList( m_buildPath+m_fileName+".report.txt", "GcbDebugger::getSubs" );

    while( true )
    {
        if( lines.isEmpty() ) break;
        QString line = lines.takeFirst();
        if( line.startsWith( "Subroutines" ) ) 
        {
            m_subs.append( lines.takeFirst() );
            break;
    }   }
    while( true )
    {
        if( lines.isEmpty() ) break;
        QString line = lines.takeFirst();
        if( line.isEmpty() ) 
        {
            if( lines.isEmpty() ) break;
            line = lines.takeFirst();
            if( !line.startsWith( ";" ) && !line.isEmpty() ) m_subs.append( line.toUpper() );
}   }   }

bool GcbDebugger::postProcess()
{
    getProcType(); // Determine Pic or Avr
    getSubs();
    bool ok = mapLstToAsm();
    if( !ok ) return false;
    return mapGcbToAsm();
}

bool GcbDebugger::mapGcbToAsm()  // Map asm_source_line <=> gcb_source_line
{
    m_varList.clear();
    m_varNames.clear();
    m_subLines.clear();

    QStringList gcbLines = fileToStringList( m_fileDir+m_fileName+".gcb", "GcbDebugger::mapGcbToAsm" );
    
    bool isFunc = false;
    int lineNum = 0;
    for( QString gcbLine : gcbLines )              // Get Declared Variables
    {
        QStringList wordList = gcbLine.split( " " );
        wordList.removeAll( "" );
        
        QString line = gcbLine;
        line = line.toUpper();
        if     ( line.startsWith( "FUNCTION" ) )     isFunc = true;
        else if( line.startsWith( "END FUNCTION" ) ) isFunc = false;
        
        for( QString word : wordList )             // Find Subs Calls
        {
            if( isFunc ) break;
            
            QString wordUp = word;
            wordUp = wordUp.toUpper();
            if( wordUp == "IF" ) break;
            
            if( m_subs.contains( wordUp ) ) { m_subLines.append( lineNum ); break; }
        }
        lineNum++;
        
        if( !line.contains( "DIM" )) continue; // Search lines containing "Dim"
        
        line    = line.replace( "'", ";" ).split( ";" ).first(); // Remove comments
        gcbLine = gcbLine.replace( "\t", " " );
        
        if( !line.contains( "AS" ))  // Should be an array
        {
            if( !line.contains( "(" )) continue;
            QStringList wordList = gcbLine.split( "(" );
            QString varName = wordList.takeFirst();
            QString varSize = wordList.takeFirst();
            
            wordList = varName.split( " " );
            wordList.removeAll( "" );
            varName = wordList.last();
            
            wordList = varSize.split( " " );
            wordList.removeAll( "" );
            varSize  = wordList.first().replace( ")", "" );
            int size = varSize.toInt();
            varSize  = QString::number( size+1 );
            
            QString type = "array"+varSize;
            if( !m_varList.contains( varName ) )
            {
                m_varList[ varName ] = type;
                m_varNames.append( varName );
        }   }
        else{
            if( wordList.first().toUpper() != "DIM" ) continue;
            if( wordList.size() < 4 ) continue;
            
            QString type = wordList.at(3).toLower();
            if( m_typesList.contains( type ) )
            {
                QString varName = wordList.at(1).toLower();
                if( !m_varList.contains( varName ) )
                {
                    m_varList[ varName ] = m_typesList[ type ];
                    m_varNames.append( varName );
    }   }   }   }
    m_flashToSource.clear();
    m_sourceToFlash.clear();
    
    QString  asmFileName = m_buildPath+m_fileName+".asm";
    QStringList asmLines = fileToStringList( asmFileName, "GcbDebugger::mapGcbToAsm" );

    bool haveVariable = false;
    int asmLineNumber = 0;
    m_lastLine = 0;
    QString srcm = ";Source:F1L";           // Gcbasic parses source lines to asm file

    for( QString asmLine : asmLines ) 
    {
        if( asmLine.contains(srcm))
        {
            asmLine.remove( srcm );
            int gcbLineNum = asmLine.split("S").first().toInt();
            
            if( gcbLineNum > m_lastLine ) m_lastLine = gcbLineNum;
            
            int asmLineNum = asmLineNumber+1;
            while( m_asmToFlash.value( asmLineNum ) == 0 ) asmLineNum++; // Avoid banksels and so
            
            int flashAddr = m_asmToFlash[asmLineNum];
            m_flashToSource[ flashAddr ]  = gcbLineNum;
        }
        else if( asmLine.contains("locations for variables")) haveVariable = true;
        else if( asmLine.contains(";*"))                      haveVariable = false;
        else if( haveVariable & (asmLine != "") )
        {
            QStringList text;
            if( m_processorType == 1 )
            {
                asmLine.remove( "EQU").replace( "\t", " ");
                text = asmLine.split(" ");
            }else{
                asmLine.remove( ".EQU").remove("\t").remove(" ");
                text = asmLine.split("=");
            }
            QString name = text.first().toLower();
            int  address = text.last().toInt();
            QString type = "uint8";
            if( m_varList.contains( name ) ) type = m_varList[ name ];
            McuInterface::self()->addWatchVar( name, address ,type  );
            
            if( type.contains( "array" ) )
            {
                m_varNames.removeOne( name );
                m_varNames.append( name );
                
                QString ty = type;
                int size = ty.replace( "array", "" ).toInt();
                for( int i=1; i<size; i++ )
                {
                    QString elmName = name+"("+QString::number( i )+")";
                    McuInterface::self()->addWatchVar( elmName, address+i ,"uint8"  );
                    if( !m_varList.contains( elmName ) ) 
                    {
                        m_varList[ elmName ] = m_typesList[ "uint8" ];
                        m_varNames.append( elmName );
        }   }   }   }
        asmLineNumber++;
    }
    QHashIterator<int, int> i(m_flashToSource);
    while( i.hasNext() )
    {
        i.next();
        int address    = i.key();
        int gcbLineNum = i.value();
        m_sourceToFlash[ gcbLineNum ] = address;
    }
    return !m_flashToSource.isEmpty();
}

bool GcbDebugger::mapLstToAsm()
{
    m_flashToAsm.clear();
    m_asmToFlash.clear();

    QString asmFile = m_buildPath+m_fileName+".asm";
    QString lstFile = m_buildPath+m_fileName+".lst";
    if( !QFileInfo::exists( m_firmware) )
    {
        m_outPane->appendLine( "\n"+QObject::tr("Error: lst file doesn't exist:")+"\n"+lstFile );
        return false;
    }

    QStringList asmLines = fileToStringList( asmFile, "GcbDebugger::mapLstToAsm" );
    QStringList lstLines = fileToStringList( lstFile, "GcbDebugger::mapLstToAsm" );

    QString asmLine;
    int asmLineNumber = 0;
    int lastAsmLine = asmLines.size();

    for( QString asmLine : asmLines ) // Go to the program start in asm file
    {
        if( asmLine.contains( "BASPROGRAMSTART" )) break;
        asmLineNumber++;
    }
    bool hasCeroAddr = false; // Deal with Banksel addr =0
    for( QString line : lstLines )
    {
        if( !line.startsWith("0") ) continue; // Code lines start with address

        line = line.replace("\t", " ").toUpper();
        line = line.remove(" ");
        line = line.split(";").first();

        while( true )
        {
            if( ++asmLineNumber >= lastAsmLine ) break; // End of asm file
            asmLine = asmLines.at( asmLineNumber ).toUpper();
            asmLine = asmLine.replace("\t", " ").remove(" ");
            if( asmLine.isEmpty() ) continue;
            if( asmLine.startsWith("_")) continue;
            if( asmLine.startsWith(";")) continue;
            if( asmLine.startsWith("#")) continue;
            if( asmLine.startsWith(".")) continue;

            asmLine = asmLine.split(";").first();
            if( line.contains(asmLine) ) break;
        }
        if( asmLineNumber >= lastAsmLine ) { asmLineNumber = 0; continue; } // End of asm file

        QString numberText = line.left( 6 ); // first 6 digits in lst file is address
        bool ok = false;
        int address = numberText.toInt( &ok, 16 )*m_processorType;  // AVR: adress*2: instruc = 2 bytes
        if( ok )
        {
            if( address==0 )                 // Deal with Banksel addr =0
            {
                if( hasCeroAddr ) continue;
                hasCeroAddr = true;
            }
            m_flashToAsm[address] = asmLineNumber;
    }   }
    QHashIterator<int, int> i(m_flashToAsm);
    while( i.hasNext() )
    {
        i.next();
        int address       = i.key();
        int asmLineNumber = i.value();
        m_asmToFlash[asmLineNumber] = address;
    }
    return !m_flashToAsm.isEmpty();
}

void GcbDebugger::getProcType()
{
    QStringList lines = fileToStringList( m_buildPath+m_fileName+".asm", "CodeEditor::getProcType" );

    for( QString line : lines )
    {
        line = line.toLower();
        if( line.contains("include") )  // Find info in included header
        {
            line.remove("include").remove("#").remove(" ").remove("<").remove(">");
            if(line.startsWith("p")) m_processorType = 1; // if mcu name starts with p then is Pic
            else                     m_processorType = 2; // Avr
            break;
}   }   }

