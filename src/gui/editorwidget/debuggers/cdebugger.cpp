/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDirIterator>

#include "cdebugger.h"
#include "utils.h"

cDebugger::cDebugger( CodeEditor* parent, OutPanelText* outPane )
         : BaseDebugger( parent, outPane )
{
    m_langLevel = 1;

    m_typesList["char"]   = "int8";
    m_typesList["uchar"]  = "uint8";
    m_typesList["byte"]   = "uint8";
    m_typesList["int"]    = "int16";
    m_typesList["uint"]   = "uint16";
    m_typesList["short"]  = "int16";
    m_typesList["ushort"] = "uint16";
    m_typesList["word"]   = "uint16";
    m_typesList["long"]   = "int32";
    m_typesList["ulong"]  = "uint32";
    m_typesList["float"]  = "float32";
    m_typesList["double"] = "float32";
}
cDebugger::~cDebugger(){}

void cDebugger::preProcess()
{
    QStringList lines = fileToStringList( m_file, "BaseDebugger::preProcess" );
    getInfoInFile( lines.first() );

    QDirIterator it( m_fileDir, {"*.c", "*.cpp", "*.h"}, QDir::Files, QDirIterator::Subdirectories);
    while( it.hasNext() ) m_fileList.append( it.next() );

    m_varTypes.clear();

    for( QString line : lines )          // Get Variables from file
    {
        line = line.replace( "\t", " " ).remove(";");
        QStringList wordList= line.split( " " );
        wordList.removeAll( "" );

        if( wordList.isEmpty() ) continue;

        QString type = wordList.takeFirst();
        if( type == "unsigned" ) type = "u"+wordList.takeFirst();

        if( m_typesList.contains( type ) )
            for( QString word : wordList )
        {
            for( QString varName : word.split(",") )
            {
                if( varName.isEmpty() ) continue;
                varName.remove(" ");
                if( !m_varTypes.contains( varName ) )
                    m_varTypes[ varName ] = m_typesList[ type ];
                //qDebug() << "cDebugger::getData  variable "<<type<<varName<<m_typesList[ type ];
}   }   }   }

