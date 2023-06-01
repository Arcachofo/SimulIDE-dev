/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>
#include <QTimer>

#include "editorwindow.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "mcu.h"
#include "simulator.h"
#include "compiler.h"
#include "utils.h"

#include "gcbdebugger.h"
#include "inodebugger.h"
#include "b16asmdebugger.h"
#include "avrgccdebugger.h"
#include "xc8debugger.h"
#include "sdccdebugger.h"
#include "picasmdebugger.h"
#include "asdebugger.h"

EditorWindow* EditorWindow::m_pSelf = NULL;

EditorWindow::EditorWindow( QWidget* parent )
            : EditorWidget( parent )
{
    m_pSelf = this;

    m_debugDoc = NULL;
    m_debugger = NULL;

    m_state     = DBG_STOPPED;
    m_stepOver  = false;

    CodeEditor::readSettings();
    loadCompilers();
}
EditorWindow::~EditorWindow(){}

void EditorWindow::updateStep()
{
    if( !m_updateScreen ) return;

    loadFile( m_debugFile );
    CodeEditor* ce = getCodeEditor();
    ce->setDebugLine( m_debugLine );
    if( m_jumpToBrk ) ce->updateScreen();
    else              ce->update();
    //m_debugDoc->updateScreen();
    m_updateScreen = false;
    m_jumpToBrk = false;
}

bool EditorWindow::upload()
{
    return uploadFirmware( false );
}

bool EditorWindow::uploadFirmware( bool debug )
{
    CodeEditor* ce = getCodeEditor();
    if( !ce ) return false;

    bool ok = ce->compile( debug );
    if( ok ) ok = ce->getCompiler()->upload();

    return ok;
}

void EditorWindow::debug()
{
    m_outPane.appendLine( "-------------------------------------------------------\n" );
    m_outPane.appendLine( tr("Starting Debbuger...")+"\n" );
    initDebbuger();
}

void EditorWindow::run()
{ 
    setStepActs( false );

    if( m_state == DBG_STOPPED ) return;
    m_state = DBG_RUNNING;
    m_debugger->run();
    CircuitWidget::self()->resumeDebug();
}

void EditorWindow::step()
{ 
    setStepActs( false );
    stepDebug();
}

void EditorWindow::stepOver()
{
    setStepActs( false );
    stepDebug( true );
}

void EditorWindow:: pause()
{
    if( m_state < DBG_STEPING )  return;
    CircuitWidget::self()->pauseDebug();

    m_resume = m_state;
    m_state  = DBG_PAUSED;

    m_debugger->pause();

    setStepActs( true );
}

void EditorWindow::reset()
{
    //CircuitWidget::self()->powerCircOff();
    Simulator::self()->stopSim();

    m_debugDoc->setDebugLine( 1 );
    m_debugDoc->updateScreen();

    m_lastCycle = 0;
    m_state = DBG_PAUSED;

    CircuitWidget::self()->powerCircDebug( true );
}

void EditorWindow::stop()
{ 
    stopDebbuger();
    m_outPane.appendLine( "\n"+tr("Debugger Stopped ")+"\n" );
    m_debuggerToolBar->setVisible( false );
    m_compileToolBar->setVisible( true );
    m_editorToolBar->setVisible( true);
}

void EditorWindow::initDebbuger()
{
    m_debugDoc = NULL;
    m_debugger = NULL;
    m_state = DBG_STOPPED;
    m_jumpToBrk = false;
    bool ok = uploadFirmware( true );

    if( ok )  // OK: Start Debugging
    {
        m_debugDoc = getCodeEditor();
        m_debugger = m_debugDoc->getCompiler();
        m_debugDoc->startDebug();

        stepOverAct->setVisible( true /*m_stepOver*/ );
        eMcu::self()->setDebugging( true );
        reset();

        m_outPane.appendLine("\n"+tr("Debugger Started")+"\n");
        m_editorToolBar->setVisible( false );
        m_compileToolBar->setVisible( false );
        m_debuggerToolBar->setVisible( true );

        runAct->setEnabled( true );
        stepAct->setEnabled( true );
        stepOverAct->setEnabled( true );
        resetAct->setEnabled( true );
        pauseAct->setEnabled( false );

        Simulator::self()->addToUpdateList( this );
    }else{
        m_outPane.appendLine( "\n"+tr("Error Starting Debugger")+"\n" );
        stopDebbuger();
    }
}

void EditorWindow::stepDebug( bool over )
{
    if( m_state == DBG_RUNNING ) return;

    if( m_debugger->stepFromLine( over ) )
    {
        m_state = DBG_STEPING;
        CircuitWidget::self()->resumeDebug();
    }else{                                // First step to PC = 0
        setStepActs( true );
        m_jumpToBrk = true;
        m_updateScreen = true;
    }
}

void EditorWindow::lineReached( codeLine_t line ) // Processor reached PC related to source line
{
    if( m_state == DBG_RUNNING ) // Check if there is a breakpoint
    {
        if( m_fileList.contains( line.file ) )
        {
            CodeEditor* ce = (CodeEditor*)m_fileList.value( line.file );
            if( !ce->getBreakPoints()->contains( line.lineNumber ) ) return;
        }
        else return;
    }
    pause();
    m_jumpToBrk = true;
}

void EditorWindow::pauseAt( codeLine_t line )
{
    m_debugFile = line.file;
    m_debugLine = line.lineNumber;

    int cycle = eMcu::self()->cycle();
    double time = Simulator::self()->circTime()/1e6;
    m_outPane.appendLine( tr("Line ")+QString::number( m_debugLine )+"\t"
                         +tr("Clock Cycles: ")+QString::number( cycle-m_lastCycle )+"\t"
                         +tr("Time us: ")+QString::number( time-m_lastTime ));
    m_lastCycle = cycle;
    m_lastTime = time;
    m_updateScreen = true;
}

void EditorWindow::stopDebbuger()
{
    if( m_state > DBG_STOPPED )
    {
        CircuitWidget::self()->powerCircOff();
        if( eMcu::self() ) eMcu::self()->setDebugging( false );

        m_state = DBG_STOPPED;
        m_debugDoc->stopDebug();

        Simulator::self()->remFromUpdateList( this );
    }
    m_debugger = NULL;
}

BaseDebugger* EditorWindow::createDebugger( QString name, CodeEditor* ce , QString code )
{
    BaseDebugger* debugger = NULL;
    QString type = m_compilers.value( name ).type;
    QString file = m_compilers.value( name ).file;
    if( type.isEmpty() )
    {
        type = m_assemblers.value( name ).type;
        file = m_assemblers.value( name ).file;
    }
    if     ( type == "arduino")  debugger = new InoDebugger( ce, &m_outPane );
    else if( type == "avrgcc" )  debugger = new AvrGccDebugger( ce, &m_outPane );
    else if( type == "xc8" )     debugger = new Xc8Debugger( ce, &m_outPane );
    else if( type == "sdcc" )    debugger = new SdccDebugger( ce, &m_outPane );
    else if( type == "gcbasic" ) debugger = new GcbDebugger( ce, &m_outPane );
    else if( type == "ascript" ) debugger = new asDebugger( ce, &m_outPane );
    //else if( type == "gputils" ) debugger = new PicAsmDebugger( ce, &m_outPane );
    //else if( type == "b16asm" )  debugger = new B16AsmDebugger( ce, &m_outPane );
    else
    {
        debugger = new BaseDebugger( ce, &m_outPane );
        if( name != "None" ) code = type.right( 2 );
        debugger->setLstType( code.right( 1 ).toInt() );
        debugger->setLangLevel( code.left( 1 ).toInt() );
    }
    if( name != "None" ) debugger->loadCompiler( file );
    return debugger;
}

void EditorWindow::loadCompilers()
{
    QString compilsPath = MainWindow::self()->getUserFilePath("codeeditor/compilers/compilers");
    loadCompilerSet( compilsPath, &m_compilers );
    compilsPath = MainWindow::self()->getUserFilePath("codeeditor/compilers/assemblers");
    loadCompilerSet( compilsPath, &m_assemblers );

    compilsPath = MainWindow::self()->getFilePath("data/codeeditor/compilers/compilers");
    loadCompilerSet( compilsPath, &m_compilers );
    compilsPath = MainWindow::self()->getFilePath("data/codeeditor/compilers/assemblers");
    loadCompilerSet( compilsPath, &m_assemblers );
}

void EditorWindow::loadCompilerSet( QString compilsPath, QMap<QString, compilData_t>* compList )
{
    if( compilsPath.isEmpty() ) return;

    QDir compilsDir = QDir( compilsPath );
    if( !compilsDir.exists() ) return;

    compilsDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compilsDir.entryList( QDir::Files, QDir::Name );
    if( xmlList.isEmpty() ) return;                  // No compilers to load
    qDebug() <<"\n"<<tr("    Loading Compilers at:")<<"\n"<<compilsPath<<"\n";

    for( QString compilFile : xmlList )
    {
        QString compilFilePath = compilsDir.absoluteFilePath( compilFile );
        if( !compilFilePath.isEmpty( ))
        {
            QDomDocument domDoc = fileToDomDoc( compilFilePath, "EditorWindow::loadCompilers");
            if( domDoc.isNull() ) continue;

            QDomElement el = domDoc.documentElement();
            if( (el.tagName() == "compiler")
              && el.hasAttribute( "name" )
              && el.hasAttribute( "type" )  )
            {
                QString compiler = el.attribute( "name" ) ;
                if( compList->contains( compiler ) ) continue;

                compilData_t compilData;
                compilData.file = compilFilePath;
                compilData.type = el.attribute( "type" ) ;

                compList->insert( compiler, compilData );
                qDebug() << tr("        Found Compiler: ") << compiler;
            }
            else qDebug() << tr("Error Loading Compiler at:") <<"\n" << compilFilePath <<"\n";
}   }   }

#include  "moc_editorwindow.cpp"
