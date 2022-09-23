/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "memory.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"
#include "memtable.h"
#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* Memory::construct( QObject* parent, QString type, QString id )
{ return new Memory( parent, type, id ); }

LibraryItem* Memory::libraryItem()
{
    return new LibraryItem(
        tr( "Ram/Rom" ),
        tr( "Logic/Memory" ),
        "2to3g.png",
        "Memory",
        Memory::construct );
}

Memory::Memory( QObject* parent, QString type, QString id )
      : LogicComponent( parent, type, id )
      , MemData()
{
    m_width  = 4;
    m_height = 11;

    m_otherPin.resize( 3 );
    m_WePin = new IoPin( 180, QPoint( 0,0 ), m_id+"-Pin_We", 0, this, input );
    m_WePin->setLabelText( "WE" );
    m_WePin->setLabelColor( QColor( 0, 0, 0 ) );
    m_WePin->setInverted( true );
    m_otherPin[0] = m_WePin;
    
    m_CsPin = new IoPin(  0, QPoint( 0,0 ), m_id+"-Pin_Cs", 0, this, input );
    m_CsPin->setLabelText( "CS" );
    m_CsPin->setLabelColor( QColor( 0, 0, 0 ) );
    m_CsPin->setInverted( true );
    m_otherPin[1] = m_CsPin;
    
    m_oePin = new IoPin( 180, QPoint( 0,0 ), m_id+"-Pin_outEnable" , 0, this, input );
    m_oePin->setLabelText( "OE" );
    m_oePin->setLabelColor( QColor( 0, 0, 0 ) );
    m_oePin->setInverted( true );
    m_otherPin[2] = m_oePin;

    m_asynchro = false;
    m_dataBytes = 1;
    m_addrBits = 0;
    m_dataBits = 0;
    setAddrBits( 8 );
    setDataBits( 8 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new IntProp<Memory>(  "Address_Bits", tr("Address Size"),"_Bits", this, &Memory::addrBits,   &Memory::setAddrBits, "uint" ),
new IntProp<Memory>(  "Data_Bits"   , tr("Data Size")   ,"_Bits", this, &Memory::dataBits,   &Memory::setDataBits, "uint" ),
new BoolProp<Memory>( "Persistent"  , tr("Persistent")  ,""     , this, &Memory::persistent, &Memory::setPersistent ),
new BoolProp<Memory>( "Inverted"    , tr("Invert Outputs"),""   , this, &Memory::invertOuts, &Memory::setInvertOuts ),
new BoolProp<Memory>( "Asynch"      , tr("Asynchronous")  ,""   , this, &Memory::asynchro,   &Memory::setAsynchro )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
    addPropGroup( { "Hidden", {
new StringProp<Memory>( "Mem","","", this, &Memory::getMem, &Memory::setMem)
    }} );
}
Memory::~Memory(){}

void Memory::stamp()                   // Called at Simulation Start
{
    m_we = true;
    m_cs = true;
    if( !m_persistent ) m_ram.fill( 0 );

    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this, m_asynchro );

    m_WePin->changeCallBack( this );
    m_CsPin->changeCallBack( this );
    m_oePin->changeCallBack( this );

    write( true );

    LogicComponent::stamp();
}

void Memory::updateStep()
{
    if( m_changed )
    {
        for( IoPin* pin : m_inPin  ) pin->changeCallBack( this, m_asynchro );
        for( IoPin* pin : m_outPin ) pin->changeCallBack( this, m_asynchro && m_we );
        m_changed = false;
    }
    if( m_memTable ) m_memTable->updateTable( &m_ram );
}

void Memory::voltChanged()        // Some Pin Changed State, Manage it
{
    bool CS = m_CsPin->getInpState();
    if( CS != m_cs ){
        m_cs = CS;
        LogicComponent::enableOutputs( CS && m_outEnable );
    }
    if( !CS ) return;

    updateOutEnabled();

    m_we = m_WePin->getInpState();
    m_address = 0;
    for( int i=0; i<m_addrBits; ++i )        // Get Address
    {
        bool state = m_inPin[i]->getInpState();
        if( state ) m_address += pow( 2, i );
    }
    if( m_we ){                             // Write
        write( true );
        Simulator::self()->addEvent( m_propDelay*m_propSize, this );
    }
    else{                                  // Read
        write( false );
        m_nextOutVal = m_ram[m_address];
        //IoComponent::sheduleOutPuts( this );
        Simulator::self()->addEvent( m_propDelay*m_propSize, this );
}   }

void Memory::runEvent()
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
    }
    else{ //IoComponent::runOutputs();
        for( int i=0; i<m_dataBits; ++i )
        {
            bool state = m_nextOutVal & (1<<i);
            bool oldst = m_outPin[i]->getOutState();
            if( state == oldst ) continue;

            m_outPin[i]->setOutStatFast( state );
        }
    }
}

void Memory::write( bool w )
{
    m_write = w;
    for( IoPin* pin : m_outPin )
    {
        if( m_outEnable ) pin->setPinMode( w ? input : output );
        else              LogicComponent::enableOutputs( false );

        if( m_asynchro ) pin->changeCallBack( this, w );
    }
    Simulator::self()->cancelEvents( this );
}

void Memory::setAsynchro( bool a )
{
    m_asynchro = a;
    m_changed = true;
}

void Memory::setMem( QString m )
{
    if( m.isEmpty() ) return;
    MemData::setMem( &m_ram, m );
}

QString Memory::getMem()
{
    QString m;
    if( !m_persistent ) return m;
    return MemData::getMem( &m_ram );
}

void Memory::updatePins()
{
    int h = m_addrBits+1;
    if( m_dataBits > h ) h = m_dataBits;
    
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
    
    m_CsPin->setPos( QPoint( 24,origY+8+h*8 ) );        // CS
    m_CsPin->isMoved();
    
    m_oePin->setPos( QPoint(-24,origY+8+h*8 ) );        // OE
    m_oePin->isMoved();
    
    m_area   = QRect( -(m_width/2)*8, origY, m_width*8, m_height*8 );
}

void Memory::setAddrBits( int bits )
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

void Memory::createAddrBits( int bits )
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

void Memory::deleteAddrBits( int bits )
{ LogicComponent::deletePins( &m_inPin, bits ); }

void Memory::setDataBits( int bits )
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

void Memory::createDataBits( int bits )
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

void Memory::deleteDataBits( int bits )
{ LogicComponent::deletePins( &m_outPin, bits ); }

void Memory::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else{
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

void Memory::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load data") );
    connect( loadAction, SIGNAL(triggered()),
                   this, SLOT(loadData()), Qt::UniqueConnection );

    QAction* saveAction = menu->addAction(QIcon(":/save.png"), tr("Save data") );
    connect( saveAction, SIGNAL(triggered()),
                   this, SLOT(saveData()), Qt::UniqueConnection );

    QAction* showEepAction = menu->addAction(QIcon(":/save.png"), tr("Show Memory Table") );
    connect( showEepAction, SIGNAL(triggered()),
                      this, SLOT(showTable()), Qt::UniqueConnection );

    menu->addSeparator();
}

void Memory::loadData()
{
    MemData::loadData( &m_ram, false, m_dataBits );
    if( m_memTable ) m_memTable->setData( &m_ram, m_dataBytes );
}

void Memory::saveData() { MemData::saveData( &m_ram, m_dataBits ); }

void Memory::showTable()
{
    MemData::showTable( m_ram.size(), m_dataBytes );
    if( m_persistent ) m_memTable->setWindowTitle( "ROM: "+idLabel() );
    else               m_memTable->setWindowTitle( "RAM: "+idLabel() );
    m_memTable->setData( &m_ram, m_dataBytes );
}

#include "moc_memory.cpp"
