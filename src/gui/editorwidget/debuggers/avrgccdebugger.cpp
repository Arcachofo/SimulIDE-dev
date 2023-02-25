/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert( QStringLiteral("LANG"), QStringLiteral("C") );
    env.insert( QStringLiteral("LC_MESSAGES"), QStringLiteral("C") );
    m_compProcess.setProcessEnvironment( env );
    m_addrBytes = 1; // Default for avr-gcc
}
AvrGccDebugger::~AvrGccDebugger(){}

bool AvrGccDebugger::postProcess()
{
    m_elfPath = m_buildPath+m_fileName+".elf";
    if( !QFileInfo::exists( m_elfPath ) )
    {
        m_outPane->appendLine( "\n"+QObject::tr("Warning: elf file doesn't exist:")+"\n"+m_elfPath );
        return false;
    }
    m_elfPath = addQuotes( m_elfPath );

    bool ok = getVariables();
    if( !ok ) return false;
    ok = getFunctions();
    if( !ok ) return false;

    m_flashToSource.clear();
    //m_sourceToFlash.clear();
    return mapFlashToSource();
}

bool AvrGccDebugger::getVariables()
{
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
            m_outPane->appendLine( "\nWarning: avr-objdump executable not detected:\n"+objdump );
    }
    m_outPane->appendText( "\nSearching for variables... " );
    objdump = addQuotes( objdump );

    QProcess getBss( NULL );      // Get var addresses from .bss section
    QString command  = objdump+" -t -j.bss "+m_elfPath;
    getBss.start( command );
    getBss.waitForFinished(-1);

    QString  p_stdout = getBss.readAllStandardOutput();
    QStringList varNames = m_varTypes.keys();
    QStringList varList;
    //m_subs.clear();

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
    m_outPane->appendLine( QString::number( varList.size() )+" variables found" );
    return true;
}

bool AvrGccDebugger::getFunctions()
{
    // avr-readelf -s file.elf
    //   Num:    Valor  Tam  Tipo    Unión  Vis      Nombre Ind
    //    34: 00000090    22 FUNC    GLOBAL DEFAULT    2 function_name
    //    27: 00800100     1 OBJECT  GLOBAL DEFAULT    3 variable_name

    QString readelf = m_toolPath+"avr/bin/avr-readelf";

#ifndef Q_OS_UNIX
    readelf += ".exe";
#endif

    if( !checkCommand( readelf ) )
    {
        readelf = m_toolPath+"avr-readelf";
    #ifndef Q_OS_UNIX
        readelf += ".exe";
    #endif
        if( !checkCommand( readelf ) )
            m_outPane->appendLine( "\nWarning: avr-readelf executable not detected:\n"+readelf );
    }
    m_outPane->appendText( "\nSearching for Functions... " );
    readelf = addQuotes( readelf );

    QProcess getFunctions( NULL );      //
    QString command  = readelf+" -s "+m_elfPath;
    getFunctions.start( command );
    getFunctions.waitForFinished(-1);

    QString  p_stdout = getFunctions.readAllStandardOutput();

    for( QString line : p_stdout.split("\n") )
    {
        if( line.isEmpty() ) continue;
        QStringList words = line.split(" ");
        words.removeAll("");
        if( words.size() < 8 ) continue;
        if( words.at(3) != "FUNC" ) continue;

        QString addr   = words.at(1);
        bool ok = false;
        int address = addr.toInt( &ok, 16 )/2;
        if( !ok ) continue;
        QString funcName = words.last();

        m_functions[funcName] = address;
        //qDebug() << "AvrGccDebugger::getFunctions "<< funcName <<address;
    }
    m_outPane->appendLine( QString::number( m_functions.size() )+" functions found" );
    return true;
}

bool AvrGccDebugger::mapFlashToSource()
{
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
            m_outPane->appendLine( "\nWarning: avr-size executable not detected:\n"+avrSize );
    }
    if( !checkCommand( addr2li ) )
    {
        addr2li = m_toolPath+"avr-addr2line";
    #ifndef Q_OS_UNIX
        addr2li += ".exe";
    #endif
        if( !checkCommand( addr2li ) )
            m_outPane->appendLine( "\nWarning: avr-addr2line executable not detected:\n"+addr2li );
    }
    m_outPane->appendText( "\nMapping Flash to Source... " );
    avrSize = addQuotes( avrSize );
    addr2li = addQuotes( addr2li );

    QProcess getSize( this );  // Get Firmware size
    getSize.start( avrSize + " " + m_elfPath );
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
    flashToLine.start( addr2li + " -e " + m_elfPath );
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

        //m_lastLine = 0;
        for( int flashAddr=0; flashAddr<flashSize; ++flashAddr ) // Map Flash Address to Source Line
        {
            QString p_stdout = flashToLine.readLine();
            if( p_stdout.isEmpty() ) continue;
            if( p_stdout.startsWith("?") ) continue;

            int idx = p_stdout.lastIndexOf( ":" );
            if( idx == -1 ) continue;


            QString filePath = QFileInfo( p_stdout.left( idx ) ).filePath();//  .fileName();
            if( m_fileList.contains( filePath ) )
            {
                bool ok = false;
                int line = p_stdout.mid( idx+1 ).toInt( &ok );
                if( !ok ) continue;
                int addr = flashAddr*m_addrBytes/2;
                //qDebug() << QString::number(flashAddr,16)<<addr<<line <<" -----> "<< p_stdout;

                setLineToFlash( {filePath,line}, addr );
        }   }
        flashToLine.close();
    }
    ok = !m_flashToSource.isEmpty();
    m_outPane->appendLine( QString::number( m_flashToSource.size() )+" lines mapped" );
    return ok;
}

