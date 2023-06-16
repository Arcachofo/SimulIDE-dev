#include <math.h>

#include "dynamic_memory.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"
#include "memtable.h"
#include "utils.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"
#include <QDebug>

Component* DynamicMemory::construct( QObject* parent, QString type, QString id )
{ return new DynamicMemory( parent, type, id ); }

LibraryItem* DynamicMemory::libraryItem()
{
    return new LibraryItem(
        tr( "Dynamic Ram" ),
        "Memory",
        "2to3g.png",
        "DynamicMemory",
        DynamicMemory::construct );
}

DynamicMemory::DynamicMemory( QObject* parent, QString type, QString id )
             : LogicComponent( parent, type, id )
             , MemData()
{
    m_width  = 4;
    m_height = 11;

    m_otherPin.resize( 4 );
    m_WePin = new IoPin( 180, QPoint( 0,0 ), m_id+"-Pin_We", 0, this, input );
    m_WePin->setLabelText( "WE" );
    m_WePin->setLabelColor( QColor( 0, 0, 0 ) );
    m_WePin->setInverted( true );
    m_otherPin[0] = m_WePin;
    
    m_RasPin = new IoPin(  0, QPoint( 0,0 ), m_id+"-Pin_Ras", 0, this, input );
    m_RasPin->setLabelText( "RAS" );
    m_RasPin->setLabelColor( QColor( 0, 0, 0 ) );
    m_RasPin->setInverted( true );
    m_otherPin[1] = m_RasPin;

    m_CasPin = new IoPin(  0, QPoint( 0,0 ), m_id+"-Pin_Cas", 0, this, input );
    m_CasPin->setLabelText( "CAS" );
    m_CasPin->setLabelColor( QColor( 0, 0, 0 ) );
    m_CasPin->setInverted( true );
    m_otherPin[2] = m_CasPin;

    m_oePin = new IoPin( 180, QPoint( 0,0 ), m_id+"-Pin_outEnable" , 0, this, input );
    m_oePin->setLabelText( "OE" );
    m_oePin->setLabelColor( QColor( 0, 0, 0 ) );
    m_oePin->setInverted( true );
    m_otherPin[3] = m_oePin;

    m_dataBytes = 1;
    m_rowAddrBits = 0;
    m_colAddrBits = 0;
    m_addrBits = 0;
    m_dataBits = 0;
    m_refreshPeriod = 0;
    setRowAddrBits( 8 );
    setColAddrBits( 8 );
    setDataBits( 8 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new IntProp <DynamicMemory>("Row_Bits"   , tr("Row Address Size")   ,"_Bits", this, &DynamicMemory::rowAddrBits,   &DynamicMemory::setRowAddrBits,0,"uint" ),
new IntProp <DynamicMemory>("Column_Bits", tr("Column Address Size"),"_Bits", this, &DynamicMemory::colAddrBits,   &DynamicMemory::setColAddrBits,0,"uint" ),
new IntProp <DynamicMemory>("Data_Bits"  , tr("Data Size")          ,"_Bits", this, &DynamicMemory::dataBits,      &DynamicMemory::setDataBits,0,"uint" ),
new DoubProp<DynamicMemory>("Refresh"    , tr("Refresh period")     ,"ps"   , this, &DynamicMemory::refreshPeriod, &DynamicMemory::setRefreshPeriod )
    },groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges")   , IoComponent::edgeProps(),0 } );
}
DynamicMemory::~DynamicMemory(){}

void DynamicMemory::stamp()                   // Called at Simulation Start
{
    m_rowAddress = 0;
    m_intRefresh = 0;
    m_refreshError = false;
    m_ras = false;
    m_cas = false;
    m_we = false;
    for( int &data : m_ram ) data = rand() % (int)( pow( 2, m_dataBits ) );

    m_WePin->changeCallBack( this );
    m_RasPin->changeCallBack( this );
    m_CasPin->changeCallBack( this );
    m_oePin->changeCallBack( this );

    write( false );

    LogicComponent::stamp();
}

void DynamicMemory::updateStep()
{
    if( m_memTable ) m_memTable->updateTable( &m_ram );
    if ( m_refreshError )
        m_error = true;
}

void DynamicMemory::voltChanged()        // Some Pin Changed State, Manage it
{
    bool RAS = m_RasPin->getInpState();
    if( RAS != m_ras ) {
        m_ras = RAS;
        if ( m_ras == true ) {                                                      // falling edge at /RAS
            int refreshRow;
            if ( !m_cas ) {                                                         // RAS before CAS - row refresh from address bus
                m_rowAddress = 0;
                for( int i=0; i<m_rowAddrBits; ++i )                                // Get Row Address
                {
                    bool state = m_inPin[i]->getInpState();
                    if( state ) m_rowAddress += pow( 2, i );
                }
                refreshRow = m_rowAddress;
                qDebug() << "Row address" << m_rowAddress;
            } else {                                                                // RAS after CAS - internal refresh
                qDebug() << "Internal refresh row" << m_rowAddress;
                refreshRow = m_intRefresh++;
                if( m_intRefresh >= pow( 2, m_rowAddrBits ) )
                    m_intRefresh = 0;
            }
            if ( Simulator::self()->circTime() > ( m_rowLastRefresh[refreshRow] + m_refreshPeriod ) ) {
                m_refreshError = true;
                qDebug() << "Row" << refreshRow << "of dynamic RAM " + idLabel() + " has not been refreshed for" << ( Simulator::self()->circTime() - m_rowLastRefresh[refreshRow] ) / 1e9 << "ms";
            }
            m_rowLastRefresh[refreshRow] = Simulator::self()->circTime();
        }
    }

    bool CAS = m_CasPin->getInpState();
    if( CAS != m_cas ) {
        m_cas = CAS;
        LogicComponent::enableOutputs( CAS && m_outEnable );
        
        if ( m_cas == true )                                                        // falling edge at /CAS
            if ( m_ras )                                                            // store address when /RAS is active
            {
                m_address = m_rowAddress * pow( 2, m_rowAddrBits );
                for( int i=0; i<m_colAddrBits; ++i )                                // Get Address
                {
                    bool state = m_inPin[i]->getInpState();
                    if( state ) m_address += pow( 2, i );
                }
                qDebug() << "Address" << m_address;
            }
    }
    
    if ( !CAS ) {
        m_we = false;
        return;
    }

    updateOutEnabled();
    if ( m_outEnable == true ) {
        m_nextOutVal = m_ram[m_address];
        IoComponent::sheduleOutPuts( this );
    }

    bool WE = m_WePin->getInpState();
    if ( WE != m_we ) {
        m_we = WE;
        if ( m_we == true ) {                                                       // falling edge at /WE - Write
            int value = 0;
            for( uint i=0; i<m_outPin.size(); ++i )
            {
                bool state = m_outPin[i]->getInpState();
                if( state ) value += pow( 2, i );
            }
            m_ram[m_address] = value;
            qDebug() << "Write" << value << "to address" << m_address;
        }
    }

}

void DynamicMemory::runEvent()
{
    if( m_write )
    {
        int value = 0;
        for( uint i=0; i<m_outPin.size(); ++i )
        {
            bool state = m_outPin[i]->getInpState();
            if( state ) value += pow( 2, i );
            m_outPin[i]->setPinState( state? input_high:input_low ); // High-Low colors
        }
        m_ram[m_address] = value;
        qDebug() << "Write" << value << "to address" << m_address;
    }
    else{ IoComponent::runOutputs();
        qDebug() << "Read from address" << m_address;
    }
}

void DynamicMemory::write( bool w )
{
    m_write = w;
    for( IoPin* pin : m_outPin )
    {
        if( m_outEnable ) pin->setPinMode( w ? input : output );
        else              LogicComponent::enableOutputs( false );

    }
    Simulator::self()->cancelEvents( this );
}

void DynamicMemory::updatePins()
{
    int h = m_addrBits+1;
    if( m_dataBits > m_addrBits ) h = m_dataBits + 1;
    
    m_height = h+2;
    int origY = -(m_height/2)*8;
    
    for( int i=0; i<m_addrBits; i++ )
    {
        m_inPin[i]->setPos( QPoint(-24,origY+8+i*8 ) );
        m_inPin[i]->isMoved();
    }
    for( int i=0; i<m_dataBits; i++ )
    {
        m_outPin[i]->setPos( QPoint(24,origY+8+i*8 ) ); 
        m_outPin[i]->isMoved();
    }
    m_WePin->setPos( QPoint(-24,origY+h*8 ) );          // WE
    m_WePin->isMoved();
    
    m_RasPin->setPos( QPoint( 24,origY+h*8 ) );         // RAS
    m_RasPin->isMoved();

    m_CasPin->setPos( QPoint( 24,origY+8+h*8 ) );       // CAS
    m_CasPin->isMoved();
    
    m_oePin->setPos( QPoint(-24,origY+8+h*8 ) );        // OE
    m_oePin->isMoved();
    
    m_area   = QRect( -(m_width/2)*8, origY, m_width*8, m_height*8 );
}

void DynamicMemory::setRowAddrBits( int rowAddrBits )
{
    int bits;
    
    if( rowAddrBits == m_rowAddrBits ) return;    
    if( rowAddrBits == 0 ) rowAddrBits = 8;
    if( ( rowAddrBits + m_colAddrBits ) > 24 ) rowAddrBits = 24 - m_colAddrBits;
    
    bits = (rowAddrBits > m_colAddrBits) ? rowAddrBits : m_colAddrBits;
    setAddrBits( bits );
    m_rowAddrBits = rowAddrBits;

    m_ram.resize( pow( 2, m_rowAddrBits + m_colAddrBits ) );  // this is the second resize, because there is the first resize in function setAddrBits
    m_rowLastRefresh.resize( pow( 2, m_rowAddrBits ) );
}

void DynamicMemory::setColAddrBits( int colAddrBits )
{
    int bits;
    
    if( colAddrBits == m_colAddrBits ) return;
    if( colAddrBits == 0 ) colAddrBits = 8;
    if( ( m_rowAddrBits + colAddrBits ) > 24 ) colAddrBits = 24 - m_rowAddrBits;

    bits = (m_rowAddrBits > colAddrBits) ? m_rowAddrBits : colAddrBits;
    setAddrBits( bits );
    m_colAddrBits = colAddrBits;
    
    m_ram.resize( pow( 2, m_rowAddrBits + m_colAddrBits ) );
}

void DynamicMemory::setAddrBits( int bits )
{
    if( bits == m_addrBits ) return;
    if( bits == 0 ) bits = 8;
    if( bits > 24 ) bits = 24;

    m_ram.resize( pow( 2, bits ) );
    
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    
    if     ( bits < m_addrBits ) deleteAddrBits( m_addrBits-bits );
    else if( bits > m_addrBits ) createAddrBits( bits-m_addrBits );
    m_addrBits = bits;

    if( m_memTable ) m_memTable->setData( &m_ram, m_dataBytes );

    updatePins();
    Circuit::self()->update();
}

void DynamicMemory::createAddrBits( int bits )
{
    int chans = m_addrBits + bits;
    int origY = -(m_height/2)*8;
    
    m_inPin.resize( chans );
    
    for( int i=m_addrBits; i<chans; i++ )
    {
        QString number = QString::number(i);

        m_inPin[i] = new IoPin( 180, QPoint(-24,origY+8+i*8 ), m_id+"-in"+number, i, this, input );
        m_inPin[i]->setLabelText( "A"+number );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        initPin( m_inPin[i] );
}   }

void DynamicMemory::deleteAddrBits( int bits )
{ LogicComponent::deletePins( &m_inPin, bits ); }

void DynamicMemory::setDataBits( int bits )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    if( bits == m_dataBits ) return;
    if( bits == 0 ) bits = 8;
    if( bits > 32 ) bits = 32;

    if     ( bits < m_dataBits ) deleteDataBits( m_dataBits-bits );
    else if( bits > m_dataBits ) createDataBits( bits-m_dataBits );
    
    m_dataBits = bits;
    m_dataBytes = m_dataBits/8;
    if( m_dataBits%8) m_dataBytes++;
    if( m_memTable ) m_memTable->setData( &m_ram, m_dataBytes );
    updatePins();
    Circuit::self()->update();
}

void DynamicMemory::createDataBits( int bits )
{
    int chans = m_dataBits + bits;
    int origY = -(m_height/2)*8;
    
    m_outPin.resize( chans );
    
    for( int i=m_dataBits; i<chans; i++ )
    {
        QString number = QString::number(i);
        
        m_outPin[i] = new IoPin( 0, QPoint(24,origY+8+i*8 ), m_id+"-out"+number, i, this, output );
        m_outPin[i]->setLabelText( "D"+number);
        m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        initPin( m_outPin[i] );
}   }

void DynamicMemory::deleteDataBits( int bits )
{ LogicComponent::deletePins( &m_outPin, bits ); }

void DynamicMemory::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.svg"),tr("Load data") );
    connect( loadAction, &QAction::triggered,
                   this, &DynamicMemory::loadData, Qt::UniqueConnection );

    QAction* saveAction = menu->addAction(QIcon(":/save.png"), tr("Save data") );
    connect( saveAction, &QAction::triggered,
                   this, &DynamicMemory::saveData, Qt::UniqueConnection );

    QAction* showEepAction = menu->addAction(QIcon(":/save.png"), tr("Show Memory Table") );
    connect( showEepAction, &QAction::triggered,
                      this, &DynamicMemory::slotShowTable, Qt::UniqueConnection );

    menu->addSeparator();
    Component::contextMenu( event, menu );
}

void DynamicMemory::loadData()
{
    MemData::loadData( &m_ram, false, m_dataBits );
    if( m_memTable ) m_memTable->setData( &m_ram, m_dataBytes );
}

void DynamicMemory::saveData() { MemData::saveData( &m_ram, m_dataBits ); }

void DynamicMemory::slotShowTable()
{
    MemData::showTable( m_ram.size(), m_dataBytes );
    m_memTable->setWindowTitle( "DRAM: "+idLabel() );
    m_memTable->setData( &m_ram, m_dataBytes );
}
