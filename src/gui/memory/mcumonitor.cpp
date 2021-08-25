#include "mcumonitor.h"
#include "simulator.h"
#include "mcuinterface.h"
#include "memtable.h"
#include "utils.h"

#define BYTES_PER_LINE 16

MCUMonitor::MCUMonitor( QWidget* parent, McuInterface* mcu )
          : QDialog( parent )
{
    setupUi(this);
    hide();
    m_processor = mcu;

    m_ramTable = m_processor->getRamTable();
    m_status = &m_ramTable->m_status;
    m_pc     = &m_ramTable->m_pc;
    m_pc->setMinimumWidth( 130 );

    horizontalLayout->insertWidget( 0, m_status);
    horizontalLayout->insertWidget( 0, m_pc);
    horizontalLayout->setStretchFactor( m_pc, 30 );
    horizontalLayout->setStretchFactor( m_status, 65 );

    m_ramMonitor   = new MemTable( tabWidget, m_processor->ramSize() );
    m_flashMonitor = new MemTable( tabWidget, m_processor->flashSize(), m_processor->wordSize() );
    m_romMonitor   = new MemTable( tabWidget, m_processor->romSize() );

    m_ramTable->getSplitter()->insertWidget( 2, m_ramMonitor );
    m_ramTable->getSplitter()->setSizes( {50,320,50} );

    tabWidget->addTab( m_ramTable, "RAM" );
    //tabWidget->addTab( m_ramMonitor, "RAM");
    tabWidget->addTab( m_romMonitor, "EEPROM");
    tabWidget->addTab( m_flashMonitor, "Flash");

    connect( tabWidget,      SIGNAL(currentChanged(int)),   this, SLOT(tabChanged(int)) );
    connect( m_ramMonitor,   SIGNAL(dataChanged(int, int)), this, SLOT(ramDataChanged(int, int)));
    connect( m_flashMonitor, SIGNAL(dataChanged(int, int)), this, SLOT(flashDataChanged(int, int)));
    connect( m_romMonitor,   SIGNAL(dataChanged(int, int)), this, SLOT(eepromDataChanged(int, int)));
}

void MCUMonitor::ramDataChanged( int address, int val )
{ m_processor->setRamValue( address, val ); }

void MCUMonitor::flashDataChanged( int address, int val )
{ m_processor->setFlashValue( address, val ); }

void MCUMonitor::eepromDataChanged(  int address, int val )
{ m_processor->setRomValue( address, val ); }

void MCUMonitor::tabChanged( int )
{
    if( Simulator::self()->isRunning() ) return;
    if( tabWidget->count() < 3 ) return;
    updateStep();
}

void MCUMonitor::updateStep()
{
    int status = m_processor->status();
    for( int i=0; i<8; i++ )
    {
        int bit = status & 1;
        if( bit ) m_status->item( 0, 7-i )->setBackground( QColor( 255, 150, 00 ) );
        else      m_status->item( 0, 7-i )->setBackground( QColor( 120, 230, 255 ) );
        status >>= 1;
    }

    int pc = m_processor->pc();
    m_pc->item( 0, 0 )->setData( 0, pc );
    m_pc->item( 0, 1 )->setText(" 0x"+decToBase(pc, 16, 4).remove(0,1) );

    if( m_ramTable->isVisible() )
    {
        if( m_ramTable->getSplitter()->sizes().at(1) > 150 ) // RAM MemTable visible
            m_ramTable->updateValues();
        if( m_ramTable->getSplitter()->sizes().at(2) > 100 ) // RAM MemTable visible
        {
            for( uint32_t i=0; i<m_processor->ramSize(); ++i )
                m_ramMonitor->setValue( i, m_processor->getRamValue(i));
    }   }
    else if( m_romMonitor->isVisible() ) m_romMonitor->setData( m_processor->eeprom() );
    else if( m_flashMonitor->isVisible() )
    {
        for( uint32_t i=0; i<m_processor->flashSize(); ++i )
            m_flashMonitor->setValue( i, m_processor->getFlashValue(i));
    }
}
