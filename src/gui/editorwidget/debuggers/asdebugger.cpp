/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileInfo>

#include "asdebugger.h"
#include "outpaneltext.h"
#include "scriptcpu.h"
#include "mcu.h"
#include "codeeditor.h"

#define tr(str) QCoreApplication::translate("Compiler",str)

asDebugger::asDebugger( CodeEditor* parent, OutPanelText* outPane )
          : BaseDebugger( parent, outPane )
{
    m_device = nullptr;

    remProperty( "ToolPath" );

    //QStringList keywords;
    //keywords << "component.addCpuReg";
    //parent->addKeyWords( keywords );
}
asDebugger::~asDebugger()
{
    if( m_device ) m_device->setDebugger( nullptr );
}

bool asDebugger::upload() // Copy hex file to Circuit folder, then upload
{
    if( !m_device )
    {
        m_outPane->appendText( "\n"+tr("Error uploading Script to ") );
        m_outPane->appendLine( Mcu::self()->device() );
        return false;
    }
    m_debugStep = false;
    m_stepOver = false;
    m_running = false;
    eMcu::self()->setDebugger( this );
    //m_device->startScript();
    m_outPane->appendText( "\n"+tr("Script Uploaded to ") );
    m_outPane->appendLine( Mcu::self()->device() );
    return true;
}

int asDebugger::compile( bool )
{
    m_firmware = m_buildPath+m_fileName+m_fileExt;
    m_device = nullptr;

    if( !m_firmware.isEmpty() && !QFileInfo::exists( m_firmware ) )
    {
        m_outPane->appendLine( "\n"+tr("Error: script file doesn't exist:")+"\n"+m_firmware );
        return -1;
    }

    Mcu* mcu = Mcu::self();
    if( !mcu || !mcu->isScripted() )
    {
        m_outPane->appendLine("\n"+tr("Error: No Scripted Device Found... ") );
        return -1;
    }
    m_device = static_cast<ScriptCpu*>( mcu->cpu() );
    m_device->setDebugger( this );
    m_device->setScriptFile( m_firmware, false );
    int r = m_device->compileScript();
    if( r == 0 )
    {
        m_editor->setExtraTypes( m_device->getTypes() );
        m_editor->setMemberWords( m_device->getMemberWords() );
        m_outPane->appendLine( "\n"+tr("     SUCCESS!!! Compilation Ok")+"\n" );
    }
    else m_outPane->appendLine( "\n"+tr("     ERROR!!! Compilation Failed")+"\n" );

    return r;
}

void asDebugger::scriptError( int line )
{
    m_editor->addError( line );
}
void asDebugger::scriptWarning( int line )
{
    m_editor->addWarning( line );
}
