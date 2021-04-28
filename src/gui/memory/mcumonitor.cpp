#include "mcumonitor.h"
#include "simulator.h"
#include "mcuinterface.h"
#include "memtable.h"

#define BYTES_PER_LINE 16

MCUMonitor::MCUMonitor( QWidget* parent, McuInterface* mcu )
          : QDialog( parent )
{
    setupUi(this);
    hide();
    m_processor = mcu;

    m_ramTable = m_processor->getRamTable();
    verticalLayout->insertWidget( 0, &m_ramTable->m_status);
    verticalLayout->insertWidget( 0, &m_ramTable->m_pc);

    m_ramMonitor    = new MemTable( tabWidget, m_processor->ramSize() );
    m_flashMonitor  = new MemTable( tabWidget, m_processor->flashSize() );
    m_romMonitor    = new MemTable( tabWidget, m_processor->romSize() );

    tabWidget->addTab( m_ramTable, "Variables" );
    tabWidget->addTab( m_ramMonitor, "RAM");
    tabWidget->addTab( m_romMonitor, "EEPROM");
    tabWidget->addTab( m_flashMonitor, "Flash");

    connect( tabWidget,      SIGNAL(currentChanged(int)),   this, SLOT(tabChanged(int)) );
    connect( m_ramMonitor,   SIGNAL(dataChanged(int, int)), this, SLOT(ramDataChanged(int, int)));
    connect( m_flashMonitor, SIGNAL(dataChanged(int, int)), this, SLOT(flashDataChanged(int, int)));
    connect( m_romMonitor,   SIGNAL(dataChanged(int, int)), this, SLOT(eepromDataChanged(int, int)));
}

void MCUMonitor::updateStep()
{ updateTable( tabWidget->currentIndex() ); }

void MCUMonitor::ramDataChanged( int address, int val )
{ m_processor->setRamValue( address, val ); }

void MCUMonitor::flashDataChanged( int address, int val )
{ m_processor->setFlashValue( address, val ); }

void MCUMonitor::eepromDataChanged(  int address, int val )
{ m_processor->setRomValue( address, val ); }

void MCUMonitor::tabChanged( int index )
{
    if( Simulator::self()->isRunning() ) return;
    if( tabWidget->count() < 4 ) return;
    updateTable( index );
}

void MCUMonitor::updateTable( int index )
{
    switch( index )
    {
    case 0:
        m_ramTable->updateValues();
        break;
    case 1:
        for( int i=0; i<m_processor->ramSize(); ++i )
            m_ramMonitor->setValue( i, m_processor->getRamValue(i));
        break;
    case 2:
        m_romMonitor->setData( m_processor->eeprom() );
        break;
    case 3:
        for( int i=0; i<m_processor->flashSize(); ++i )
            m_flashMonitor->setValue( i, m_processor->getFlashValue(i));
        break;
    }
}

