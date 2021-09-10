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

#include <QDomDocument>

#include "editorwindow.h"
#include "mainwindow.h"
#include "simulator.h"
#include "compiler.h"
#include "utils.h"
#include "simuapi_apppath.h"

#include "gcbdebugger.h"
#include "inodebugger.h"
#include "b16asmdebugger.h"
#include "avrgccdebugger.h"
#include "sdccdebugger.h"
#include "avrasmdebugger.h"
#include "picasmdebugger.h"

EditorWindow* EditorWindow::m_pSelf = NULL;

EditorWindow::EditorWindow( QWidget* parent )
            : EditorWidget( parent )
{
    m_pSelf = this;
    CodeEditor::readSettings();
    loadCompilers();
}
EditorWindow::~EditorWindow(){}

void EditorWindow::debug()
{ 
    CodeEditor* ce = getCodeEditor();

    if( ce->initDebbuger() )
    {
        m_editorToolBar->setVisible( false);
        m_debuggerToolBar->setVisible( true );

        runAct->setEnabled( true );
        stepAct->setEnabled( true );
        stepOverAct->setEnabled( true );
        resetAct->setEnabled( true );
        pauseAct->setEnabled( false );

        Simulator::self()->addToUpdateList( &m_outPane );
}   }

void EditorWindow::run()
{ 
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( runToBreak() ) );
}

void EditorWindow::step()    
{ 
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( step()) );
}

void EditorWindow::stepOver()
{
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( stepOver()) );
}

void EditorWindow::pause()   
{
    getCodeEditor()->pause();
    runAct->setEnabled( true );
    stepAct->setEnabled( true );
    stepOverAct->setEnabled( true );
    resetAct->setEnabled( true );
    pauseAct->setEnabled( false );
}

void EditorWindow::stop()    
{ 
    getCodeEditor()->stopDebbuger();
    m_debuggerToolBar->setVisible( false );
    m_editorToolBar->setVisible( true);

    Simulator::self()->remFromUpdateList( &m_outPane );
}

BaseDebugger* EditorWindow::createDebugger( QString name, CodeEditor* ce )
{
    BaseDebugger* debugger = NULL;
    QString type = m_compilers.value( name ).type;

    if     ( type == "arduino")  debugger = new InoDebugger( ce, &m_outPane );
    else if( type == "avrgcc" )  debugger = new AvrGccDebugger( ce, &m_outPane );
    else if( type == "sdcc" )    debugger = new SdccDebugger( ce, &m_outPane );
    else if( type == "gcbasic" ) debugger = new GcbDebugger( ce, &m_outPane );
    else if( type == "picasm" )  debugger = new PicAsmDebugger( ce, &m_outPane );
    else if( type == "avrasm" )  debugger = new AvrAsmDebugger( ce, &m_outPane );
    else if( type == "b16asm" )  debugger = new B16AsmDebugger( ce, &m_outPane );
    else                         debugger = new BaseDebugger( ce, &m_outPane );

    if( name != "None" ) debugger->loadCompiler( m_compilers.value( name ).file );
    return debugger;
}


void EditorWindow::loadCompilers()
{
    QString compilsPath = SIMUAPI_AppPath::self()->availableDataFilePath("codeeditor/compilers/");
    QDir compilsDir = QDir( compilsPath );

    compilsDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compilsDir.entryList( QDir::Files );
    if( xmlList.isEmpty() ) return;                  // No compilers to load

    qDebug() << "\n" << tr("    Loading Compilers at:")<< "\n" << compilsPath<<"\n";

    for( QString compilFile : xmlList )
    {
        QString compilFilePath = compilsDir.absoluteFilePath( compilFile );

        if( !compilFilePath.isEmpty( ))  //loadXml( compilFilePath );
        {
            QDomDocument domDoc = fileToDomDoc( compilFilePath, "EditorWindow::loadCompilers");
            if( domDoc.isNull() ) continue;

            QDomElement el = domDoc.documentElement();
            if( (el.tagName() == "compiler")
              && el.hasAttribute( "name" )
              && el.hasAttribute( "type" )  )
            {
                QString compiler = el.attribute( "name" ) ;

                compilData_t compilData;
                compilData.file = compilFilePath;
                compilData.type = el.attribute( "type" ) ;

                m_compilers[compiler] = compilData;
                qDebug() << tr("Found Compiler: ") << compiler;
            }
            else qDebug() << tr("Error Loadind Compiler at:") <<"\n" << compilFilePath <<"\n";
    }   }
    qDebug() << "\n";
}

#include  "moc_editorwindow.cpp"
