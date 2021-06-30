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

#include <math.h>

#include "avrprocessor.h"
#include "mcucomponent.h"
#include "utils.h"
#include "simulator.h"

// simavr includes
#include "sim_elf.h"
#include "sim_hex.h"
#include "sim_core.h"
#include "sim_gdb.h"
#include "avr_uart.h"
#include "avr_eeprom.h"

extern "C"
int elf_read_firmware_ext( const char* file, elf_firmware_t* firmware );

AvrProcessor::AvrProcessor( McuComponent* parent )
            : BaseProcessor( parent )
{
    m_avrProcessor = NULL;
    m_avrEEPROM = NULL;
    m_initGdb = false;
    m_statusReg = "SREG";
    QStringList statusBits;
    statusBits <<" C "<<" Z "<<" N "<<" V "<<" S "<<" H "<<" T "<<" I ";
    m_ramTable->setStatusBits( statusBits );
}
AvrProcessor::~AvrProcessor() {}

void AvrProcessor::terminate()
{
    BaseProcessor::terminate();
    if( m_avrProcessor )
    {
        avr_deinit_gdb( m_avrProcessor );
        avr_terminate( m_avrProcessor );
    }
}

bool AvrProcessor::setDevice( QString device )
{
    if( m_avrProcessor ) return false;
    m_device = device;
    
    m_avrProcessor = avr_make_mcu_by_name( m_device.toUtf8().constData() );

    if( !m_avrProcessor )
    {
        MessageBoxNB( "AvrProcessor::setDevice"
                    , tr("Could not Create AVR Processor: \"%1\"").arg(m_device) );
        return false;
    }
    int started = avr_init( m_avrProcessor );

    m_uartInIrq.resize( 6 );
    m_uartInIrq.fill(0);
    for( int i=0; i<6; i++ )// Uart interface
    {
        avr_irq_t* src = avr_io_getirq( m_avrProcessor, AVR_IOCTL_UART_GETIRQ('0'+i), UART_IRQ_OUTPUT);
        if( src )
        {
            qDebug() << "    UART"<<i;
            intptr_t uart = i;
            avr_irq_register_notify( src, uart_pty_out_hook, (void*)uart ); // Irq to get data coming from AVR

            // Irq to send data to AVR:
            m_uartInIrq[i] = avr_io_getirq( m_avrProcessor, AVR_IOCTL_UART_GETIRQ('0'+i), UART_IRQ_INPUT);
        }
    }
    qDebug() << "\nAvrProcessor::setDevice Avr Init: "<< m_device << (started==0);

    m_avrProcessor->frequency = 16000000;
    m_avrProcessor->cycle = 0;

    m_ramSize   = m_avrProcessor->ramend+1;
    m_flashSize = (m_avrProcessor->flashend+1)/2;
    m_romSize   = m_avrProcessor->e2end+1;

    avr_eeprom_desc_t ee;
    ee.ee = 0;
    ee.offset = 0;
    ee.size = m_romSize;
    int ok = avr_ioctl( m_avrProcessor, AVR_IOCTL_EEPROM_GET, &ee );
    if( ok ) m_avrEEPROM = ee.ee;
    m_eeprom.resize( m_romSize );
    return true;
}

bool AvrProcessor::loadFirmware( QString fileN )
{
    if( !m_avrProcessor ) return false;
    if( fileN == "" ) return false;
    QFileInfo fileInfo( fileN );

    if( fileInfo.completeSuffix().isEmpty() ) fileN.append(".hex");

    if( !QFile::exists( fileN ) )     // File not found
    {
        QMessageBox::warning( 0, tr("File Not Found"),
                                 tr("The file \"%1\" was not found.").arg(fileN) );
        return false;
    }

    char name[20] = "";
    strncpy( name, m_device.toUtf8(), sizeof(name)-1 );
    *(name + sizeof(name) -1) = 0;

    char filename[1000]="";
    strncpy( filename, fileN.toUtf8(), sizeof(filename)-1 );
    *(filename + sizeof(filename) -1) = 0;

    elf_firmware_t f = {{0}};

    if( fileN.endsWith("hex") )
    {
        ihex_chunk_p chunk = NULL;
        int cnt = read_ihex_chunks( filename, &chunk );

        if( cnt <= 0 )
        {
            QMessageBox::warning(0,tr("Error:"), tr(" Unable to load IHEX file %1\n").arg(fileN) );
            return false;
        }

        int lastFChunk = 0;

        for( int ci=0; ci<cnt; ci++ )
        {
            if( chunk[ci].baseaddr < (1*1024*1024) )
            {
                if( chunk[ci].baseaddr > chunk[lastFChunk].baseaddr) lastFChunk = ci;
            }
        }
        f.flashbase = chunk[ 0 ].baseaddr;
        f.flashsize = chunk[ lastFChunk ].baseaddr + chunk[ lastFChunk ].size;
        f.flash = (uint8_t*) malloc( f.flashsize+1 );

        for( int ci=0; ci<cnt; ci++ )
        {
            if( chunk[ci].baseaddr < (1*1024*1024) )
            {
                memcpy( f.flash + chunk[ci].baseaddr,
                        chunk[ci].data,
                        chunk[ci].size );
            }
            if( chunk[ci].baseaddr >= AVR_SEGMENT_OFFSET_EEPROM )
            {
                f.eeprom = chunk[ci].data;
                f.eesize = chunk[ci].size;
            }
        }
        free_ihex_chunks(chunk);
    }
    else if( fileN.endsWith(".elf") )
    {
        f.flashsize = 0;
        elf_read_firmware_ext( filename, &f );
        
        if( !f.flashsize )
        {
            QMessageBox::warning(0,tr("Failed to load firmware: "), tr("File %1 is not in valid ELF format\n").arg(fileN) );
            return false;
        }
    }
    else                                    // File extension not valid
    {
        QMessageBox::warning(0,tr("Error:"), tr("%1 should be .hex or .elf\n").arg(fileN) );
        return false;
    }

    QString mmcu( f.mmcu );
    if( !mmcu.isEmpty() )
    {
        if( mmcu != m_device ) 
        {
            QMessageBox::warning(0,tr("Warning on load firmware: "), tr("Incompatible firmware: compiled for %1 and your processor is %2\n").arg(mmcu).arg(m_device) );
            return false;
        }
    }else{
        if( !strlen( name ) )
        {
            QMessageBox::warning( 0,tr("Failed to load firmware: "), tr("The processor model is not specified.\n") );
            return false;
        }
        strcpy( f.mmcu, name );
    }

    if( avr_load_firmware( m_avrProcessor, &f ) != 0 )
    {
        QMessageBox::warning(0,tr("Error:"), tr("Wrong firmware!!").arg(f.mmcu) );
        return false;
    }
    if( f.flashbase ) m_avrProcessor->pc = f.flashbase;

    m_firmware = fileN;
    m_loadStatus = true;

    return true;
}

void AvrProcessor::stepCpu()
{
    if( m_avrProcessor->state < cpu_Done ) m_avrProcessor->run( m_avrProcessor );
    else m_mcu->crash();
}

void AvrProcessor::reset()
{
    if( !m_loadStatus ) return;
    
    for( int i=0; i<m_avrProcessor->ramend; i++ ) m_avrProcessor->data[i] = 0;

    avr_reset( m_avrProcessor );
    m_avrProcessor->pc = 0;
    m_avrProcessor->cycle = 0;

    if( m_resetStatus /*|| m_debugging*/ ) return;
    Simulator::self()->addEvent( 1, this );
}

int AvrProcessor::pc()
{ return m_avrProcessor->pc/2; }

uint8_t AvrProcessor::getRamValue( int address )
{ return m_avrProcessor->data[address]; }

void AvrProcessor::setRamValue( int address, uint8_t value )
{ m_avrProcessor->data[address] = value; }

uint16_t AvrProcessor::getFlashValue( int address )
{
    address *= 2;
    return( m_avrProcessor->flash[address]
         | (m_avrProcessor->flash[address+1] << 8) );
}

void AvrProcessor::setFlashValue( int address, uint16_t value )
{
    address *= 2;
    *(m_avrProcessor->flash + address)   = value & 0x00FF;
    *(m_avrProcessor->flash + address+1) = value & 0xFF00;
}

uint8_t AvrProcessor::getRomValue( int address )
{ return m_avrEEPROM[address]; }

void AvrProcessor::setRomValue( int address, uint8_t value )
{ *(m_avrEEPROM + address) = value; }

int AvrProcessor::validate( int address )
{
    if( address < 64 ) address += 32;
    return address;
}

void AvrProcessor::uartIn( int uart, uint32_t value ) // Receive one byte on Uart
{
    if( !m_avrProcessor ) return;

    avr_irq_t* uartInIrq = m_uartInIrq[uart];
    if( uartInIrq )
    {
        BaseProcessor::uartIn( uart, value );
        avr_raise_irq( uartInIrq, value );
    }
}

bool AvrProcessor::initGdb()
{
    return m_initGdb;
}

void AvrProcessor::setInitGdb( bool init )
{
    m_initGdb = init;
    if( m_initGdb )
    {
        m_avrProcessor->gdb_port = 1212;
        int ok = avr_gdb_init( m_avrProcessor );
        if( ok < 0 )
        {
            m_avrProcessor->gdb_port = 0;
            qDebug() << "avr_gdb_init ERROR " << ok;
        }
        else qDebug() << "avr gdb Initialized at IP: 127.0.0.1  PORT: 1212";
    }
    else
    {
        m_avrProcessor->gdb_port = 0;
        qDebug() << "avr gdb Disabled";
    }
}

void AvrProcessor::setRegisters()
{
    BaseProcessor::setRegisters();

    for( int i=0; i<32; i++ )       // GPRs
    {
        QString name = "R"+QString::number( i );
        addWatchVar( name, i, "u8" );
    }
}

#include "moc_avrprocessor.cpp"

