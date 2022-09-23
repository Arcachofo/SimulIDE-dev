/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileInfo>

#include "gcbdebugger.h"
#include "e_mcu.h"
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

bool GcbDebugger::postProcess()
{
    getProcType(); // Determine Pic or Avr
    getSubs();
    bool ok = mapFlashToSource();
    return ok;
}

void GcbDebugger::getSubs()
{
    QStringList lines = fileToStringList( m_buildPath+m_fileName+".report.txt", "GcbDebugger::getSubs" );

    bool read = false;
    QString funcName;
    QString line;
    while( !lines.isEmpty() )
    {
        line = lines.takeFirst();
        if( line.startsWith( "Subroutines" ) ) read = true;
        if( !read ) continue;
        funcName = "";
        while( !lines.isEmpty() )
        {
            line = lines.takeFirst();
            if( line.startsWith(";") ) break;
            if     ( line.isEmpty()     ) funcName = "";
            else if( funcName.isEmpty() ) funcName = line.toUpper();
            else if( line.startsWith("Compiled Size (words):") )
                m_functions[funcName] = 0;
        }
    }
}

bool GcbDebugger::mapFlashToSource()  // Map asm_source_line <=> gcb_source_line
{
    QStringList varList;
    m_varTypes.clear();

    QStringList gcbLines = fileToStringList( m_fileDir+m_fileName+".gcb", "GcbDebugger::mapFlashToSource" );

    int lineNum = 0;
    for( QString gcbLine : gcbLines )              // Get Declared Variables
    {
        QStringList wordList = gcbLine.split( " " );
        wordList.removeAll( "" );
        
        QString line = gcbLine;
        line = line.toUpper();
        lineNum++;
        
        if( !line.contains("DIM") ) continue; // Search lines containing "Dim"
        
        line    = line.replace( "'", ";" ).split( ";" ).first(); // Remove comments
        gcbLine = gcbLine.replace( "\t", " " );
        
        if( !line.contains("AS") )  // Should be an array
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
            if( !m_varTypes.contains( varName ) )
            {
                m_varTypes[ varName ] = type;
                varList.append( varName );
        }   }
        else{
            if( wordList.first().toUpper() != "DIM" ) continue;
            if( wordList.size() < 4 ) continue;
            
            QString type = wordList.at(3).toLower();
            if( m_typesList.contains( type ) )
            {
                QString varName = wordList.at(1).toLower();
                if( !m_varTypes.contains( varName ) )
                {
                    m_varTypes[ varName ] = m_typesList[ type ];
                    varList.append( varName );
    }   }   }   }

    m_flashToSource.clear();
    m_sourceToFlash.clear();

    QString lstFile = m_buildPath+m_fileName+".lst";
    if( !QFileInfo::exists( lstFile ) )
    {
        m_outPane->appendLine( "\n"+QObject::tr("Error: list file doesn't exist:")+"\n"+lstFile );
        return false;
    }
    QStringList lstLines = fileToStringList( lstFile, "GcbDebugger::mapFlashToSource" );
    QString lstLine;
    while( lstLine != "BASPROGRAMSTART" ) // Go to the program start in lst file
    {
        lstLine = lstLines.takeFirst();
        lstLine.remove("\t");
    }
    
    QString  asmFileName = m_buildPath+m_fileName+".asm";
    QStringList asmLines = fileToStringList( asmFileName, "GcbDebugger::mapFlashToSource" );

    bool haveVariable = false;
    QString asmLine;
    while( !asmLine.contains("BASPROGRAMSTART:") )
    {
        asmLine = asmLines.takeFirst();
        if     ( asmLine.isEmpty() ) continue;
        else if( asmLine.contains("locations for variables") ) haveVariable = true;
        else if( asmLine.startsWith(";*"))                    haveVariable = false;
        else if( asmLine.startsWith(";") ) continue;
        else if( haveVariable )                                // Find Variable Addresses
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
            if( m_varTypes.contains( name ) ) type = m_varTypes.value( name );
            eMcu::self()->getRamTable()->addVariable( name, address ,type  );

            if( type.contains( "array" ) )
            {
                varList.removeOne( name );
                varList.append( name );

                QString ty = type;
                int size = ty.replace( "array", "" ).toInt();
                for( int i=1; i<size; i++ )
                {
                    QString elmName = name+"("+QString::number( i )+")";
                    eMcu::self()->getRamTable()->addVariable( elmName, address+i ,"uint8"  );
                    if( !m_varTypes.contains( elmName ) )
                    {
                        m_varTypes[ elmName ] = m_typesList.value("uint8");
                        varList.append( elmName );
    }   }   }   }   }

    int gcbLineNum = 0;
    bool hasCeroAddr = false; // Deal with Banksel addr=0
    m_lastLine = 0;
    QString srcm = ";SOURCE:F1L";  // Gcbasic parses source lines to asm file
    QString funcName;

    for( QString asmLine : asmLines ) 
    {
        if( asmLine.isEmpty() ) continue;
        asmLine = asmLine.toUpper().replace("\t"," ");

        if( asmLine.contains( srcm ) )
        {
            gcbLineNum = asmLine.remove( srcm ).split("S").first().toInt();
        }
        else if( asmLine.startsWith(";") ) continue;
        else if( asmLine.contains( ":" ) )                      // Find Subroutines
        {
            funcName = asmLine.left( asmLine.indexOf(":") ).toUpper();
            if( !m_functions.contains( funcName ) ) funcName = "";
        }
        else{
            asmLine = asmLine.remove(0,1);
            lstLine = "";
            while( !lstLines.isEmpty() && !lstLine.contains( asmLine ) )
                lstLine = lstLines.takeFirst();

            if( gcbLineNum )
            {
                QString numberText = lstLine.left( 6 ); // first 6 digits in lst file is address
                bool ok = false;
                int flashAddr = numberText.toInt( &ok, 16 );
                if( ok ){
                    if( flashAddr == 0 )                 // Deal with Banksel addr = 0
                    {
                        if( hasCeroAddr ) continue;
                        hasCeroAddr = true;
                    }
                    setLineToFlash( gcbLineNum, flashAddr );

                    if( !funcName.isEmpty() ) // Subroutine starting here
                    {
                        m_functions[funcName] = flashAddr;
                        funcName = "";
                    }
                }
                gcbLineNum = 0;
            }
        }
    }
    eMcu::self()->getRamTable()->setVariables( varList );

    return !m_flashToSource.isEmpty();
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

