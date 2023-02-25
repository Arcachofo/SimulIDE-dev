/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QProcess>
#include <QFileInfo>

//#include <QDebug>

#include "gputilsdebug.h"
#include "e_mcu.h"
#include "basedebugger.h"
#include "outpaneltext.h"
#include "utils.h"

GputilsDebug::GputilsDebug()
{
}
GputilsDebug::~GputilsDebug(){}

/*int GputilsDebug::compile( bool debug )
{
}*/

bool GputilsDebug::getVariables( BaseDebugger* debugger )
{
    QString gpvc    = debugger->toolPath()+"gpvc";
    QString codPath = debugger->buildPath()+debugger->fileName()+".cod";
    if( !QFileInfo::exists( codPath ) )
    {
        debugger->outPane()->appendLine( "\nWarning: cod file doesn't exist:\n"+codPath );
        return false;
    }
#ifndef Q_OS_UNIX
    gpvc += ".exe";
#endif
    if( !debugger->checkCommand( gpvc ) )
        debugger->outPane()->appendLine( "\nWarning: gpvc executable not detected:\n"+gpvc );

    debugger->outPane()->appendText( "\nSearching for variables... " );
    gpvc    = addQuotes( gpvc );
    codPath = addQuotes( codPath );

    QProcess getVars( NULL );      // Get var addresses from Symbol Table
    getVars.setWorkingDirectory( debugger->buildPath() );
    QString command  = gpvc+" -s "+codPath;
    getVars.start( command );
    getVars.waitForFinished(-1);

    QString  p_stdout = getVars.readAllStandardOutput();
    QStringList varNames;

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

        QString type = debugger->getVarType( symbol );
        /*else{
            QString size = words.at(4);
            if( size.startsWith("c_") ) size.remove( 0, 2 );

            if( m_typesList.contains(size) ) type = m_typesList.value( size );
        }*/
        if( type.isEmpty() ) continue;
        eMcu::self()->getRamTable()->addVariable( symbol, address, type );
        varNames.append( symbol );
        //qDebug() << "GputilsDebug::getData  variable "<<type<<symbol<<address;
    }
    eMcu::self()->getRamTable()->setVariables( varNames );
    debugger->outPane()->appendLine( QString::number( varNames.size() )+" variables found" );
    return true;
}

bool GputilsDebug::mapFlashToSource( BaseDebugger* debugger )
{
    QString gpvc    = debugger->toolPath()+"gpvc";
    QString codPath = debugger->buildPath()+debugger->fileName()+".cod";
    if( !QFileInfo::exists( codPath ) )
    {
        debugger->outPane()->appendLine( "\nWarning: cod file doesn't exist:\n"+codPath );
        return false;
    }
    debugger->outPane()->appendText( "\nMapping Flash to Source... " );
#ifndef Q_OS_UNIX
    gpvc += ".exe";
#endif

    gpvc    = addQuotes( gpvc );
    codPath = addQuotes( codPath );

    QProcess flashToLine( NULL );      // Get var addresses from Symbol Table
    flashToLine.setWorkingDirectory( debugger->buildPath() );
    QString command  = gpvc+" -l "+codPath;
    flashToLine.start( command );
    flashToLine.waitForFinished(-1);
    QString  p_stdout = flashToLine.readAllStandardOutput();

    bool readAddr = false;
    QString lineNum;
    for( QString line : p_stdout.split("\n") )
    {
        //qDebug() << line;
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

            debugger->setLineToFlash( {debugger->file(), lineN }, addr );
            continue;
        }
        if( line.startsWith( ";") && line.contains(".line") )
        {
            QStringList words = line.split("\"");
            QString file = getFileName( words.at(1) );
            if( debugger->isProjectFile( file ) )
            {
                lineNum = words.at(0);
                lineNum = lineNum.remove(";").replace("\t"," ");
                words = lineNum.split(" ");
                words.removeAll("");
                if( words.size() < 2 ) continue;
                lineNum = words.at(1);
                readAddr = true;
    }   }   }
    debugger->outPane()->appendLine( QString::number( debugger->flashToSourceSize() )+" lines mapped" );
    return true;
}

bool GputilsDebug::mapFlashToAsm( BaseDebugger* debugger )
{
    QString gpvc    = debugger->toolPath()+"gpvc";
    QString codPath = debugger->buildPath()+debugger->fileName()+".cod";
    if( !QFileInfo::exists( codPath ) )
    {
        debugger->outPane()->appendLine( "\nWarning: cod file doesn't exist:\n"+codPath );
        return false;
    }

#ifndef Q_OS_UNIX
    gpvc += ".exe";
#endif

    gpvc    = addQuotes( gpvc );
    codPath = addQuotes( codPath );

    QProcess flashToLine( NULL );      // Get var addresses from Symbol Table
    flashToLine.setWorkingDirectory( debugger->buildPath() );
    QString command  = gpvc+" -l "+codPath;
    flashToLine.start( command );
    flashToLine.waitForFinished(-1);
    QString  p_stdout = flashToLine.readAllStandardOutput();

    bool readAddr = false;
    QString lineNum;
    for( QString line : p_stdout.split("\n") )
    {
        //qDebug() << line;
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

            debugger->setLineToFlash( {debugger->file(), lineN }, addr );
            continue;
        }
        if( line.startsWith( ";") && line.contains(".line") )
        {
            QStringList words = line.split("\"");
            QString file = getFileName( words.at(1) );
            if( debugger->isProjectFile( file ) )
            {
                lineNum = words.at(0);
                lineNum = lineNum.remove(";").replace("\t"," ");
                words = lineNum.split(" ");
                words.removeAll("");
                if( words.size() < 2 ) continue;
                lineNum = words.at(1);
                readAddr = true;
    }   }   }
    return true;
}
