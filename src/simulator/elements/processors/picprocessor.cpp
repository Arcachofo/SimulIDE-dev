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

#include <QtGui>

#include "picprocessor.h"
#include "mcucomponent.h"
#include "utils.h"
#include "simulator.h"

// GpSim includes
#include "uart.h"
#include "pir.h"
#include "eeprom.h"
#include "hexutils.h"

PicProcessor::PicProcessor( McuComponent* parent )
            : BaseProcessor( parent )
            , m_hexLoader()
{
    m_pPicProcessor = NULL;
    m_loadStatus    = false;
    m_statusReg = "STATUS"; // Staus Bits defined in PicProcessor::setDevice
}
PicProcessor::~PicProcessor(){}

void PicProcessor::setDevice( QString device )
{
    if( m_pPicProcessor ) return;
    m_device = device;

    QStringList statusBits;

    if( m_device.startsWith( "pic18" ) )
            statusBits <<" C "<<"DC "<<" Z "<<"OV "<<" N "<<" X "<<" X "<<" X ";
    else    statusBits <<" C "<<"DC "<<" Z "<<"PD "<<"TO "<<"RP0"<<"RP1"<<"IRP";

    m_ramTable->setStatusBits( statusBits );

    qDebug() << "Creating Proccessor:    "<<m_device<<"\n" ;

    Processor* p = ProcessorConstructor::CreatePic( m_device.toUtf8().constData() );
    m_pPicProcessor = dynamic_cast<pic_processor*>( p );

    if( !m_pPicProcessor )
    {
        QMessageBox::warning( 0, tr("Unkown Error:")
                               , tr("Could not Create Pic Processor: \"%1\"").arg(m_device) );
        return;
    }
    m_ramSize   = m_pPicProcessor->register_memory_size();
    m_flashSize = m_pPicProcessor->program_memory_size();

    qDebug() << "    UARTs:";
    m_rcsta.resize(6);
    m_rcsta.fill(0);
    for( int i=0; i<6; i++ ) // Get UARTs
    {
        QString rc = "RCSTA";
        if( i > 0 ) rc += QString::number(i);
        uint32_t address = getRegAddress( rc );
        if( address < m_ramSize )
        {
            int uart = i;
            if( i > 0 ) uart--;  // Uart 0 and 1 are the same
            qDebug() << "      uart"<<uart;
            _RCSTA* rcsta = dynamic_cast<_RCSTA*>(m_pPicProcessor->registers[address]);
            m_rcsta[uart] = rcsta;
            rcsta->m_picProc = this;
        }
    }
    EEPROM* eeprom = m_pPicProcessor->eeprom;
    if( eeprom )
    {
        m_romSize = eeprom->get_rom_size();
        m_eeprom.resize( m_romSize );
    }
}

bool PicProcessor::loadFirmware( QString fileN )
{
    if( !m_pPicProcessor ) return false;
    if( fileN == "" ) return false;
    
    m_firmware = fileN;
    
    if( !QFile::exists( m_firmware ) )
    {
        QMessageBox::warning( 0, tr("File Not Found")
                               , tr("The file \"%1\" was not found.").arg(m_firmware) );
        return false;
    }
    QByteArray symbolFile = m_firmware.toUtf8();
    
    m_loadStatus = false;

    qDebug() << "Loading HexFile:\n"<<m_firmware<<"\n" ;
    
    FILE* pFile  = fopen( symbolFile.constData(), "r" );
    int load = m_hexLoader.readihex16( m_pPicProcessor, pFile );
    if( load == HexLoader::SUCCESS ) m_loadStatus = true;
    
    if( !m_loadStatus )
    {
        QMessageBox::warning( 0, tr("Unkown Error:")
                               , tr("Could not Load: \"%1\"").arg(m_firmware) );
        return false;
    }
    m_pPicProcessor->set_Vdd( 5 );

    qDebug() << "\nProcessor Ready:";
    qDebug() << "    Device    =" << m_pPicProcessor->name_str;
    qDebug() << "    Int. OSC  =" << (m_pPicProcessor->get_int_osc()? "true":"false");
    qDebug() << "    Use PLLx4 =" << (m_pPicProcessor->get_pplx4_osc()? "true":"false")<<"\n";

    int address = getRegAddress( "OSCCAL" );
    if( address > 0 ) // Initialize Program Memory at 0x3FF for OSCCAL
    {
        qDebug() << "    OSCCAL    = true";
        m_pPicProcessor->init_program_memory_at_index( 0x3FF, 0x3400 );
    }
    m_loadStatus = true;
    return true;
}

int PicProcessor::pc()
{ return m_pPicProcessor->pc->get_value(); }

void PicProcessor::setFreq( double freq ) // Instruction exec. freq
{
    BaseProcessor::setFreq( freq );
    m_pPicProcessor->set_frequency( freq*1e6 );
}

void PicProcessor::reset()
{
    if( !m_loadStatus ) return;

    if( m_pPicProcessor->is_sleeping() ) m_pPicProcessor->exit_sleep();
    m_pPicProcessor->reset( POR_RESET ); // POR_RESET MCLR_RESET EXIT_RESET IO_RESET

    if( m_resetStatus /*|| m_debugging*/ ) return;
    Simulator::self()->addEvent( 1, this );
}

uint8_t PicProcessor::getRamValue( int address )
{ return m_pPicProcessor->registers[address]->get_value(); }

void PicProcessor::setRamValue( int address, uint8_t value )
{ m_pPicProcessor->registers[address]->put_value( value ); }

uint16_t PicProcessor::getFlashValue( int address )
{ return m_pPicProcessor->program_memory[address]->get_value(); }

void PicProcessor::setFlashValue( int address, uint16_t value )
{ m_pPicProcessor->program_memory[address]->put_value( value ); }

uint8_t PicProcessor::getRomValue( int address )
{ return m_pPicProcessor->eeprom->rom[address]->get_value(); }

void PicProcessor::setRomValue(int address, uint8_t value)
{ m_pPicProcessor->eeprom->rom[address]->put_value( value ); }

void PicProcessor::uartIn( int uart, uint32_t value ) // Receive one byte on Uart
{
     if( !m_pPicProcessor ) return;

    _RCSTA* rcsta = m_rcsta[uart];
    if( rcsta )
    {
        BaseProcessor::uartIn( uart, value );
        rcsta->queueData( value );
    }
}

#include "moc_picprocessor.cpp"

