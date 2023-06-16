/***************************************************************************
 *   Copyright( C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include <QDebug>
#include <QPainter>
#include <QPushButton>
#include <QGraphicsProxyWidget>

#include "ds18b20.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "utils.h"

#include "doubleprop.h"
#include "stringprop.h"

Component* Ds18b20::construct( QObject* parent, QString type, QString id )
{ return new Ds18b20( parent, type, id ); }

LibraryItem* Ds18b20::libraryItem()
{
    return new LibraryItem(
        "DS18B20",
        "Sensors",
        "ic_comp.png",
        "DS18B20",
        Ds18b20::construct );
}

Ds18b20::Ds18b20( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( id )
{    
    m_area = QRect(-28,-16, 56, 32 );

    srand( time(0) );
    m_state    = W1_IDLE;
    m_parPower = false;
    m_tempInc  = 0.5;
    setTemp( 22 );

    generateROM( 0x28 );  // FAMILY CODE = 28h for DS18B20

    m_scratchpad[0] = 0xFF;
    m_scratchpad[1] = 0x07;
    m_scratchpad[2] = 0x4B; /// TH  reg, default value
    m_scratchpad[3] = 0x46; /// TL  reg, default value
    m_scratchpad[4] = 0x7F; /// CFG reg, default value
    m_scratchpad[5] = 0xFF;
    m_scratchpad[6] = 0x01;
    m_scratchpad[7] = 0x10;
    m_scratchpad[8] = 0x2F;

    m_pin.resize(1);
    m_pin[0] = m_inpin = new IoPin( 180, QPoint(-36, 8), id+"-inPin", 0, this, openCo );

    QPushButton* u_button = new QPushButton();
    u_button->setMaximumSize( 9, 9 );
    u_button->setGeometry(-5,-5, 9, 9 );
    u_button->setCheckable( false );
    u_button->setIcon(QIcon(":/su.png"));

    QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( u_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( -7, 4 ) );

    QPushButton* d_button = new QPushButton();
    d_button->setMaximumSize( 9, 9 );
    d_button->setGeometry(-5,-5, 9, 9 );
    d_button->setCheckable( false );
    d_button->setIcon( QIcon(":/giu.png") );

    proxy = Circuit::self()->addWidget( d_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( 9, 4 ) );

    connect( u_button, &QPushButton::pressed,
             this,     &Ds18b20::upbuttonclicked );

    connect( d_button, &QPushButton::pressed,
             this,     &Ds18b20::downbuttonclicked );

    m_font.setFamily("Ubuntu Mono");
    m_font.setPixelSize( 10 );
    m_font.setBold( true );
    m_font.setLetterSpacing( QFont::PercentageSpacing, 100 );
    setLabelPos(-24,-28 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new StrProp <Ds18b20>("ROM"    ,"ROM"                 ,""  , this, &Ds18b20::getROM,  &Ds18b20::setROM, propNoCopy ),
new DoubProp<Ds18b20>("Temp"   , tr("Temperature")    ,"°C", this, &Ds18b20::temp   , &Ds18b20::setTemp    ),
new DoubProp<Ds18b20>("TempInc", tr("Temp. increment"),"°C", this, &Ds18b20::tempInc, &Ds18b20::setTempInc ),
    },0} );
}
Ds18b20::~Ds18b20(){}

void Ds18b20::stamp()   // Called at Simulation Start
{
    m_lastTime = 0;
    m_rxReg   = 0;
    m_lastBit = 7;
    m_write  = false;
    m_lastIn = false;
    m_alarm  = false;

    m_inpin->setPinMode( openCo );
    m_inpin->setOutState( true );
    m_inpin->changeCallBack( this, true );
    m_state = W1_IDLE;
}

void Ds18b20::updateStep()
{
    setTemp( m_temp );
    Simulator::self()->remFromUpdateList( this );
}

void Ds18b20::voltChanged()                              // Called when Input Pin changes
{
    bool inState = m_inpin->getVoltage() > 2.5;
    uint64_t circTime = Simulator::self()->circTime();

    if( m_lastIn && !inState ) m_lastTime = circTime; // Falling edge
    else if( !m_lastIn && inState )                   // Rising edge
    {
        uint64_t time = circTime-m_lastTime; // in picoseconds

        if( time > 205*1e6 )             // > 480 us : Reset (Tested in real device 202 us)
        {
            //qDebug() <<"\n"<< idLabel() << "Ds18b20::voltChanged -------------- RESET"<<time/1e3<<"ns";
            m_rxReg = 0;
            m_bitIndex = 0;
            m_state = W1_ROM_CMD;
            m_write = false;
            pulse( 20, 80 ); // Send 80 us pulse (60 to 240 us)  after 20 us (15 to 60 us)
        }
        else if( m_state > W1_IDLE )    // Active
        {
            if( time > 30*1e6 )         // > 30 us : 0 received
            {
                if( !m_write ) readBit( 0 ); // If writting ignore long pulses
            }
            else if( time < 15*1e6 )          // < 15 us : 1 received Or Read pulse
            {
                if( m_state == W1_BUSY ){ if( m_busyTime > circTime ) pulse( 1, 50 ); } // Still busy: Send a 50 us pulse after 1 us
                else{
                    if( m_write ) writeBit();
                    else          readBit( 1 );
    }   }   }   }
    m_lastIn = inState;
}

void Ds18b20::runEvent()
{
    if( m_pullDown )   // Pull down
    {
        m_pullDown = false;
        m_inpin->sheduleState( false, 0 );
        Simulator::self()->addEvent( m_pulse, this );
    }
    else{             // Release
        m_inpin->sheduleState( true, 0 );
        m_inpin->changeCallBack( this, true );         // Receive voltChange() CallBacks again
    }
}

void Ds18b20::sendData( uint8_t data, int size )
{
    m_txReg = data;
    m_bitIndex = 0;
    m_lastBit = size-1;
    m_write = true;
}

void Ds18b20::writeBit()
{
    if( (m_txReg & 1<<m_bitIndex) == 0 ) pulse( 1, 25 ); // Send a 25 us pulse after 1 us

    if( m_bitIndex == m_lastBit ) // Byte sent
    {
        m_bitIndex = 0;
        m_write = false;
        dataSent();
    }
    else m_bitIndex += 1;
}

void Ds18b20::dataSent() // Last data has been sent
{
    //if( m_state != W1_SEARCH ) qDebug() << idLabel() << "Ds18b20::dataSent"<< val2hex( m_txReg );

    if( m_state == W1_SEARCH ) m_lastBit = 0; // Read 1 bit
    else if( !m_txBuff.empty() )   // Send next byte in Tx Buffer
    {
        m_txBuff.pop_back();  // Remove last sent byte
        if( m_txBuff.empty() )// All data sent
        {
            if     ( m_state == W1_ROM_CMD ) m_state = W1_FUN_CMD;  // Wait for Function command
            else if( m_state == W1_FUN_CMD ) m_state = W1_IDLE;     // Wait for Reset pulse
        }
        else sendData( m_txBuff.back() ); // Send last byte in list, if list is not empty
    }
}

void Ds18b20::readBit( uint8_t bit )
{
    if( m_state == W1_MATCH )  // Compare bit received with ROM
    {
        if( (bit>0) != bitROM( m_bitIndex ) )
        {
            m_state = W1_IDLE; // No ROM match, we are out
            m_rxReg = 0;
            m_bitIndex = 0;
            m_lastBit = 7;     // Return to normal byte reception
            //qDebug() <<idLabel()<<"Ds18b20::readBit       NO ROM match";
            return;
        }
    }
    if( bit ) m_rxReg |= 1<<m_bitIndex;

    if( m_bitIndex == m_lastBit )   // Complete byte received
    {
        m_bitIndex = 0;
        m_lastBit = 7;   // Return to "byte" mode
        dataReceived();
        m_rxReg = 0;
    }
    else m_bitIndex += 1;
}

void Ds18b20::dataReceived() // Complete data has been received (it's in m_rxReg)
{
    switch( m_state )
    {
        case W1_IDLE: break;                           // Error, this shoild not happen
        case W1_ROM_CMD: romCommand( m_rxReg ); break; // ROM Command received
        case W1_FUN_CMD: funCommand( m_rxReg ); break; // Function Command received
        case W1_DATA:
        {
            if( m_lastCommand == 0x4E )              // Write Scratchpad
            {
                m_scratchpad[m_byte] = m_rxReg;
                if( m_byte == 4 )
                {
                    m_scratchpad[8] = crc8( m_scratchpad, 8 );
                    m_state = W1_IDLE; // Transaction finished
                }
                else m_byte++;
            }
        } break;
        case W1_MATCH:          // ROM Match: we are online, wait for Function commands
        {
            m_state = W1_FUN_CMD;
            m_rxReg = 0;
            m_bitIndex = 0;
            m_lastBit = 7;     // Return to normal byte reception
            //qDebug() <<idLabel()<<"Ds18b20::dataReceived     ROM match";
        } break;
        case W1_SEARCH:
        {
            if( ++m_bitSearch == 64 )    // We passed Search ROM: wait for commands
            {
                m_state = W1_IDLE;
                m_lastBit = 7; // Return to normal byte reception
                //qDebug() <<idLabel()<< "Ds18b20::dataReceived  :  Search ROM OK ";
            }
            else if( (m_rxReg > 0) == m_bitROM ) sendSearchBit();   // Bit Match,  keep sending
            else{
                m_state = W1_IDLE; // We are out, Wait for next Reset signal
                //qDebug() <<idLabel()<< "Ds18b20::dataReceived  :  Search ROM OUT";
            }
        } break;
        default: qDebug() <<idLabel()<< "Ds18b20::dataReceived  :  ERROR";
    }
}

void Ds18b20::pulse( uint64_t time, uint64_t witdth ) // Time in us
{
    m_pullDown = true;
    m_pulse = witdth*1e6;                          // Keep line low for width us
    m_inpin->changeCallBack( this, false );          // Stop receiving voltChange() CallBacks
    Simulator::self()->addEvent( time*1e6, this ); // Send pulse after time us
}

// ROM COMMANDS --------------------------------------------------

void Ds18b20::romCommand( uint8_t cmd )
{
    switch( cmd )
    {
        case 0x33: readROM();   break;
        case 0x55: matchROM();  break;
        case 0xCC: skipROM();   break;
        case 0xF0: searchROM(); break;
        default:{
            m_state = W1_IDLE;
            qDebug()<<idLabel() << "Ds18b20::command : Warning: ROM command Not implemented";
        }
    }
    m_lastCommand = cmd;
}

void Ds18b20::readROM() // Code: 33h : send ROM to Master
{
    //qDebug() << idLabel() <<"Ds18b20::readROM"<< arrayToHex( m_ROM, 8 );

    m_txBuff.clear();
    for( int i=7; i>=0; i-- ) m_txBuff.push_back( m_ROM[i] );

    sendData( m_txBuff.back() );
}

void Ds18b20::matchROM() // Code 55h : read 64 bits and compare with ROM
{
    //qDebug() <<idLabel()<< "Ds18b20::matchROM";
    m_lastBit = 63;
    m_state = W1_MATCH;
}

void Ds18b20::skipROM() // Code: CCh : This device is selected, Wait command
{
    //qDebug() <<idLabel()<< "Ds18b20::skipROM";
    m_state = W1_FUN_CMD;
}

void Ds18b20::searchROM() // Code F0h
{
    //qDebug() <<idLabel()<< "Ds18b20::searchROM";
    m_state = W1_SEARCH;
    m_bitSearch = 0;
    sendSearchBit();
}

void Ds18b20::alarmSearch()
{
    if( m_alarm ) searchROM();
    else          m_state = W1_IDLE;
}

void Ds18b20::sendSearchBit()
{
    m_bitROM = bitROM( m_bitSearch );
    uint8_t data = m_bitROM ? 1 : 2;
    sendData( data, 2 );
}

bool Ds18b20::bitROM( uint bitIndex )
{
    return ( m_ROM[ bitIndex/8 ] & 1<<(bitIndex%8) ) > 0;
}

// Function COMMANDS --------------------------------------------------

void Ds18b20::funCommand( uint8_t cmd )
{
    switch( cmd ){
        case 0x44: convertTemp();     break;
        case 0x48: copyScratchpad();  break;
        case 0x4E: writeScratchpad(); break;
        case 0xB4: readPowerSupply(); break;
        case 0xB8: recallE2();        break;
        case 0xBE: readScratchpad();  break;
        default:{  m_state = W1_IDLE;
            qDebug()<<idLabel() << "Ds18b20::command : Warning: Function command Not implemented";
    }   }
    m_lastCommand = cmd;
}

void Ds18b20::convertTemp() // Code 44h, temperature already in the Scratchpad, nothing to do
{
    uint16_t temp = round( fabs( m_temp )*16 ); // Make it positive and shift to make integer value
    if( m_temp < 0 ) temp = ~temp + 1;         // Temp under 0? Make 2nd complement

    m_scratchpad[0] = temp;
    m_scratchpad[1] = temp >> 8;
    m_scratchpad[8] = crc8( m_scratchpad, 8 );

    int res = ((m_scratchpad[4] >> 5) & 0b11)+9; // Resolution
    switch( res ){                               // Resol.  Busy time
        case  9: m_busyTime =  93750*1e6; break; // 9-bits  93.75 ms
        case 10: m_busyTime = 187500*1e6; break; // 10-bit  187.5 ms
        case 11: m_busyTime = 375000*1e6; break; // 11-bit  375   ms
        case 12: m_busyTime = 750000*1e6; break; // 12-bit  750   ms
    }
    m_busyTime += Simulator::self()->circTime();
    m_state = W1_BUSY;

    //qDebug() << idLabel() <<"Ds18b20::setTemp"<<arrayToHex( m_scratchpad, 9 );
}

void Ds18b20::writeScratchpad() // Code 4Eh : Master will write TH, TL, Config
{
    //qDebug() <<idLabel()<< "Ds18b20::writeScratchpad";

    m_byte = 2;        // start writting to byte 2 of the scratchpad
    m_state = W1_DATA;
}

void Ds18b20::readScratchpad() // Code BEh : send Scratchpad LSB
{
    m_txBuff.clear();
    for( int i=8; i>=0; i-- ) m_txBuff.push_back( m_scratchpad[i] );

    //qDebug() << idLabel() <<"Ds18b20::readScratchpad :"<<arrayToHex( m_txBuff.data(), m_txBuff.size() );

    sendData( m_txBuff.back() );
}

void Ds18b20::copyScratchpad() // Code 48h : Copy TH, TL, CFG (bytes 2, 3, 4) to EEPROM.
{
    //qDebug() <<idLabel()<< "Ds18b20::copyScratchpad";

    m_TH  = m_scratchpad[2];
    m_TL  = m_scratchpad[3];
    m_CFG = m_scratchpad[4];

    m_busyTime = Simulator::self()->circTime()+10000*1e6; // 10 ms
    m_state = W1_BUSY;
}

void Ds18b20::recallE2() // Code B8h :Copy EEPROM to TH, TL, CFG (bytes 2, 3, 4)
{
    //qDebug() <<idLabel()<< "Ds18b20::recallE2";

    m_scratchpad[2] = m_TH;
    m_scratchpad[3] = m_TL;
    m_scratchpad[4] = m_CFG;
    m_scratchpad[8] = crc8( m_scratchpad, 8 );

    m_busyTime = Simulator::self()->circTime()+10*1e3*1e6; // 10 ms ????????????
    m_state = W1_BUSY;
}

void Ds18b20::readPowerSupply() // Code B4h : using parasite power? pull down time???
{
    //qDebug() <<idLabel()<< "Ds18b20::readPowerSupply No parasite power"; // By now we don't use parasite power
    /// TODO add property ?
}

// End Commands --------------------------------------------------------------------

void Ds18b20::setTemp( double t ) // This should convert temp wrote in UI
{
    m_temp = t;
    update();
}

uint8_t Ds18b20::crc8( uint8_t* addr, uint8_t len ) // DS18B20 crc8 calc
{
    uint8_t crc = 0;

    while( len--)
    {
        uint8_t inbyte = *addr++;
        for( uint8_t i=8; i; i-- )
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if( mix) crc ^= 0x8C;
            inbyte >>= 1;
    }   }
    return crc;
}

void Ds18b20::setROM( QString ROMstr )
{
    if( Circuit::self()->pasting() ) return;

    bool ok;
    QStringList lstROM = ROMstr.split(" ");
    lstROM.removeAll("");
    for( int i=0; i<8; ++i ) m_ROM[i] = lstROM.at( i ).toInt( &ok, 16 );
    //qDebug() << idLabel() <<"Ds18b20::setROM"<<arrayToHex( m_ROM, 8 );
}

void Ds18b20::generateROM( uint8_t familyCode ) // Generate unique ROM address
{
  // 8-bit CRC | 48 bit S/N | 8-BIT FAMILY CODE, MSB - LSB
  m_ROM[0] = familyCode;
  m_ROM[1] = rand();
  m_ROM[2] = rand();
  m_ROM[3] = rand();
  m_ROM[4] = rand();
  m_ROM[5] = 0x00;
  m_ROM[6] = 0x00;
  m_ROM[7] = crc8( m_ROM, 7 );

  //qDebug() << idLabel() <<"Ds18b20::generateROM"<<arrayToHex( m_ROM, 8 );
}

QString Ds18b20::arrayToHex( uint8_t* data, uint len ) // Static
{
    QString hexValue;
    for( uint i=0; i<len; i++) hexValue.append( val2hex( data[i] ) ).append(" ");
    return hexValue;
}

void Ds18b20::upbuttonclicked()
{
    m_temp += m_tempInc;
    if( m_temp > 125 ) m_temp = 125;
    if( Simulator::self()->isRunning() ) Simulator::self()->addToUpdateList( this );
    else updateStep();
}

void Ds18b20::downbuttonclicked()
{
    m_temp = m_temp - m_tempInc;
    if( m_temp < -55 ) m_temp = -55;
    if( Simulator::self()->isRunning() ) Simulator::self()->addToUpdateList( this );
    else updateStep();
}

void Ds18b20::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );

    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( QRect(-26,-14, 52, 15 ),2,2 );

    p->setFont( m_font );
    p->setPen( QColor(0, 0, 0) );
    p->drawText(-23, -3, QString::number( m_temp )+"°C" );
}
