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
#include "mcuinterface.h"
#include "editorwindow.h"
#include "simulator.h"
#include "mcubase.h"
#include "utils.h"

BaseDebugger::BaseDebugger( CodeEditor* parent, OutPanelText* outPane )
            : Compiler( parent, outPane )
{
    m_compName = "None";
    m_processorType = 0;
    m_stepOver = false;

    m_appPath = QCoreApplication::applicationDirPath();
}
BaseDebugger::~BaseDebugger( )
{
    if( McuInterface::self() ) McuInterface::self()->getRamTable()->remDebugger( this );
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

void BaseDebugger::mapFlashToSource()
{
    //getProcType(); // Determine Pic or Avr
    //getData();
    //getSubs();

    m_flashToSource.clear();
    m_sourceToFlash.clear();
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

#include "moc_basedebugger.cpp"
