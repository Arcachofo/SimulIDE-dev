/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "b16asmdebugger.h"
#include "outpaneltext.h"
#include "simulator.h"
#include "utils.h"

B16AsmDebugger::B16AsmDebugger( CodeEditor* parent, OutPanelText* outPane )
              : BaseDebugger( parent, outPane )
{
    m_codeBits = 0;
    loadCodes();
}
B16AsmDebugger::~B16AsmDebugger() {}

int B16AsmDebugger::compile( bool )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    QString file = m_fileDir+m_fileName+m_fileExt;
    QStringList lines = fileToStringList( file, "B16AsmDebugger::compile" );

    QList<int> data;
    for( int i=0; i<256; i++ ) data << m_opcodes.value( "NOP" );
    
    for( QString line : lines )
    {
        if( line.isEmpty() ) continue;

        QStringList words = line.replace("\t", " ").split( " " );
        words.removeAll(QString(""));
        if( words.size() < 2 ) continue;

        QString addr = words.takeFirst();
        QString inst = words.takeFirst().toUpper();
        QString argu = "0";

        if( !words.isEmpty() ) argu = words.takeFirst();

        bool ok = false;
        int address = addr.toInt( &ok, 10 );
        if( !ok ) continue;
        //if( address > 255 ) continue;

        ok = false;
        int arg = argu.toInt( &ok, 10 );
        if( !ok ) continue;
        //if( arg > 255 ) continue;
        arg <<= m_codeBits;

        int opcode = m_opcodes.value( inst );

        int out = opcode + arg;

        data.replace( address, out );
    }
    int i = 0;
    QString output = "";
    for( int val : data )
    {
        QString sval = QString::number( val );
        while( sval.length() < 4) sval.prepend( " " );
        output += sval;
        if( i == 15 )
        {
            output += "\n";
            i = 0;
        }
        else
        {
            output += ",";
            i++;
        }
    }
    QString fileName = file;                    // Save Subcircuit

    fileName.replace( file.lastIndexOf( ".sac" ), 4, ".data" );
    QFile outFile( fileName );

    if( !outFile.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "B16AsmDebugger::compile",
          QObject::tr("Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
    }
    QTextStream toFile( &outFile );
    toFile.setCodec("UTF-8");
    toFile << output;
    outFile.close();
    
    QApplication::restoreOverrideCursor();
    return 0;
}

void B16AsmDebugger::loadCodes()
{
    if( m_toolPath.isEmpty() ) return;

    QStringList lines = fileToStringList( m_toolPath+"opcodes", "B16AsmDebugger::loadCodes" );
    QString li = "";

    while( li.isEmpty() )
    {
        if( lines.isEmpty() ) return;
        li = lines.takeFirst();
        li.replace( " ", "" ).replace( "\t", "" );
    }
    if( !li.startsWith( "codeBits") ) return;

    QStringList words = li.split( "=" );
    QString coBi = words.takeLast();

    bool ok = false;
    m_codeBits = coBi.toInt( &ok, 10 );
    if( !ok ) return;

    for( QString line : lines )
    {
        line.replace( " ", "" ).replace( "\t", "" );
        if( line.isEmpty() ) continue;

        words = line.split( "=" );

        QString codeName = words.takeFirst();
        QString codeStr  = words.takeFirst();

        bool ok = false;
        int code = codeStr.toInt( &ok, 10 );
        if( !ok ) continue;

        qDebug() << codeName << code;

        m_opcodes.insert( codeName, code );
}   }

bool B16AsmDebugger::postProcess()
{
    return false;
}

#include "moc_b16asmdebugger.cpp"
