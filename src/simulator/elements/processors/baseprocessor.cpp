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

#include "baseprocessor.h"
#include "basedebugger.h"
#include "codeeditor.h"
#include "mcucomponent.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "utils.h"
#include "simuapi_apppath.h"
#include "simulator.h"

BaseProcessor* BaseProcessor::m_pSelf = 0l;

BaseProcessor::BaseProcessor( McuComponent* parent )
             : QObject( parent )
             , McuInterface( "baseprocessor" )
{
    m_pSelf = this;
    m_mcu = parent;

    m_debugger = NULL;

    m_loadStatus  = false;
    m_resetStatus = false;
    m_debugging   = false;
    m_firmware = "";
    m_device     = "";
}
BaseProcessor::~BaseProcessor() 
{
}

void BaseProcessor::stamp()
{
    m_debugStep = false;
}

void BaseProcessor::runEvent()
{
    if( m_debugging )
    {
        if( m_debugStep )
        {
            int lastPC = pc();
            stepCpu();
            int PC = pc();

            if( ( lastPC != PC )
            && ( m_debugger->m_flashToSource.contains( PC ) ) )
            {
                int line = m_debugger->m_flashToSource[ PC ];
                if( line != m_prevLine )
                {
                    m_debugStep = false;
                    m_debugger->m_editor->lineReached( line );
                }
            }
        }
    }
    else stepCpu();

    Simulator::self()->addEvent( m_stepPS, this );
}

void BaseProcessor::stepOne( int line )
{
    m_prevLine = line;
    m_debugStep = true;
}

void BaseProcessor::terminate()
{
    if( m_pSelf == this ) m_pSelf= NULL;
}

void BaseProcessor::setFreq( double freq ) // Instruction exec. freq
{
    m_stepPS = 1e6/freq; //1e6*m_mcuStepsPT/freq;
}

void BaseProcessor::setDebugger( BaseDebugger* deb )
{
    m_debugger = deb;
    m_ramTable->setDebugger( deb );
}

void BaseProcessor::setDataFile( QString datafile ) 
{ 
    m_dataFile = datafile;
    setRegisters();
    m_ramTable->setRegisters( m_regList );
}

int BaseProcessor::status() { return getRamValue( m_statusReg ); }

void BaseProcessor::hardReset( bool rst )
{
    m_resetStatus = rst;
    if( rst ) McuComponent::self()->reset();
}

uint16_t BaseProcessor::getRegAddress( QString name )
{
    name = name.toUpper();
    if( m_regsTable.contains( name ) ) return m_regsTable.value( name ); 
    return -1;
}

uint8_t BaseProcessor::getRamValue( QString name )
{
    if( m_regsTable.isEmpty() ) return -1;

    bool isNumber = false;
    int address = name.toInt( &isNumber );      // Try to convert to integer

    if( !isNumber ) address = m_regsTable[name.toUpper()];  // Is a register name

    return getRamValue( address );
}

void BaseProcessor::addWatchVar( QString name, int address, QString type )
{
    name = name.toUpper();
    if( !m_regsTable.contains(name) ) m_regList.append( name );
    m_regsTable[ name ] = address;
    m_typeTable[ name ] = type;
}

void BaseProcessor::setRegisters() // get register addresses from data file
{
    QStringList lineList = fileToStringList( m_dataFile, "BaseProcessor::setRegisters" );

    if( !m_regsTable.isEmpty() ) 
    {
        m_regList.clear();
        m_regsTable.clear();
        m_typeTable.clear();
    }

    for( QString line : lineList )
    {
        if( line.contains("EQU ") )   // This line contains a definition
        {
            line = line.replace("\t"," ");

            QString name    = "";
            QString addrtxt = "";
            int address   = 0;
            bool isNumber = false;

            line.remove(" ");
            QStringList wordList = line.split("EQU"); // Split in words
            if( wordList.size() < 2 ) continue;

            name    = wordList.takeFirst();
            while( addrtxt.isEmpty() ) addrtxt = wordList.takeFirst();

            address = addrtxt.toInt( &isNumber, 0 );
            
            if( isNumber )        // If found a valid address add to map
            {
                address = validate( address );
                addWatchVar( name, address, "u8" );        // type uint8 
            }
        }
    }
}

void BaseProcessor::uartOut( int uart, uint32_t value ) // Send value to OutPanelText
{
    emit uartDataOut( uart, value );
}

void BaseProcessor::uartIn( int uart, uint32_t value ) // Receive one byte on Uart
{
    emit uartDataIn( uart, value );
}

#include "moc_baseprocessor.cpp"
