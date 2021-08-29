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

#include "basedebugger.h"
#include "baseprocessor.h"
#include "editorwindow.h"
#include "mainwindow.h"
#include "simulator.h"
#include "mcubase.h"

static const char* BaseDebugger_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Drive Circuit"),
    QT_TRANSLATE_NOOP("App::Property","Compiler Path")
};

BaseDebugger::BaseDebugger( CodeEditor* parent, OutPanelText* outPane, QString filePath )
            : Compiler( parent, outPane )
{
    Q_UNUSED( BaseDebugger_properties );

    m_editor  = parent;
    m_outPane = outPane;
    m_appPath = QCoreApplication::applicationDirPath();
    
    QFileInfo fi = QFileInfo( filePath );
    m_file     = filePath;
    m_fileDir  = fi.absolutePath()+"/";
    m_fileExt  = "."+fi.suffix();
    m_fileName = fi.completeBaseName();

    m_processorType = 0;
    type = 0;
    
    connect( &m_compProcess, SIGNAL(readyRead()), SLOT(ProcRead()), Qt::UniqueConnection );
}
BaseDebugger::~BaseDebugger( )
{
    if( McuInterface::self() ) McuInterface::self()->getRamTable()->remDebugger( this );
}

int BaseDebugger::compile( bool debug )
{
    int error = Compiler::compile( m_file, debug );
    if( error == 0 ) m_firmware = m_fileDir+m_fileName+".hex";
    return error;
}

bool BaseDebugger::upload()
{
    if( !McuBase::self() )
    {
        m_outPane->appendLine( "\n"+tr("Error: No Mcu in Simulator... ") );
        return false;
    }
    bool ok = McuBase::self()->load( m_firmware );
    if( ok ) m_outPane->appendText( "\n"+tr("FirmWare Uploaded to ") );
    else     m_outPane->appendText( "\n"+tr("Error uploading firmware to ") );

    m_outPane->appendLine( McuBase::self()->device() );
    m_outPane->appendLine( m_firmware+"\n" );

    if( ok )
    {
        McuInterface::self()->setDebugger( this );
        mapFlashToSource();
    }
    return ok;
}

int BaseDebugger::getValidLine( int line )
{
    while( !m_sourceToFlash.contains(line) && line<=m_lastLine ) line++;
    return line;
}

QString BaseDebugger::getVarType( QString var )
{
    var = var.toUpper();
    return m_varList[ var ];
}

void BaseDebugger::ProcRead()
{
    while( m_compProcess.canReadLine() ) 
        m_outPane->appendText( QString::fromLocal8Bit( m_compProcess.readLine()) );
}

void BaseDebugger::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    
    if( settings->contains( m_compSetting ) )
        m_compilerPath = settings->value( m_compSetting ).toString();
}

void BaseDebugger::getCompilerPath()
{
    m_compilerPath = QFileDialog::getExistingDirectory( NULL
                         , tr("Select Compiler toolchain directory")
                         , m_compilerPath
                         , QFileDialog::ShowDirsOnly
                         | QFileDialog::DontResolveSymlinks);

    if( m_compilerPath != "" ) m_compilerPath += "/";

    MainWindow::self()->settings()->setValue( m_compSetting, m_compilerPath);

    m_outPane->appendLine( "\n"+tr("Using Compiler Path: ") );
    m_outPane->appendLine( m_compilerPath+"\n" );
}

bool BaseDebugger::driveCirc()
{
    CodeEditor* ce = EditorWindow::self()->getCodeEditor();
    return ce->driveCirc();
}

void BaseDebugger::setDriveCirc( bool drive )
{
    CodeEditor* ce = EditorWindow::self()->getCodeEditor();
    ce->setDriveCirc( drive );
}

void BaseDebugger::setCompilerPath( QString path )
{
    m_compilerPath = path;
    MainWindow::self()->settings()->setValue( m_compSetting, m_compilerPath );
}

void BaseDebugger::toolChainNotFound()
{
    m_outPane->appendLine( tr(": ToolChain not found") );
    m_outPane->appendLine( "\n"+tr("Right-Click on Document Tab to set Path")+"\n" );
    QApplication::restoreOverrideCursor();
}

#include "moc_basedebugger.cpp"
