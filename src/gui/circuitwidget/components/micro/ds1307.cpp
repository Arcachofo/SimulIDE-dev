/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "ds1307.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

Component* DS1307::construct( QObject* parent, QString type, QString id )
{ return new DS1307( parent, type, id ); }

LibraryItem* DS1307::libraryItem()
{
    return new LibraryItem(
        "DS1307",
        "Perifericals",
        "dsxxx_ico.png",
        "DS1307",
        DS1307::construct );
}

DS1307::DS1307( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , TwiModule( id )
      , m_clock( id+"_clock" )
{
    m_area = QRect(-28,-20, 56, 40 );
    m_background = ":/dsxxx.png";
    setLabelPos(-20,-34, 0 );

    m_pin.resize( 3 );
    m_pin[0] = m_pinSda = new IoPin( 180, QPoint(-36,-12), id+"-PinSDA", 0, this, openCo );
    m_pinSda->setLabelText( "SDA" );
    TwiModule::setSdaPin( m_pinSda );

    m_pin[1] = m_clkPin = new IoPin( 180, QPoint(-36, -4), id+"-PinSCL", 0, this, openCo );
    m_clkPin->setLabelText( "SCL" );
    TwiModule::setSclPin( m_clkPin );

    m_pin[2] = m_outpin = new IoPin( 180, QPoint(-36, 12), id+"-PinSQW", 0, this, output );
    m_outpin->setLabelText( "SQW" );
    m_outpin->setOutHighV( 5 );
    m_clock.setPin( m_outpin );

    m_timeUpdtd = true;

    m_address = m_cCode = 0b01101000; // 0x68

    addPropGroup( { "Main", {
new BoolProp<DS1307>( "Time_Updtd", tr("Set current time at start"),"", this, &DS1307::timeUpdtd, &DS1307::setTimeUpdtd ),
        },0 } );
}
DS1307::~DS1307(){}

void DS1307::initialize()
{
    TwiModule::initialize();

    m_phase = 0;
    for( int i=0; i<64; i++ ) m_data[i] = 0x00;

    if( m_timeUpdtd )
    {
        m_clock.disable( false );
        m_clock.setCurrentTime();
    }else{
        m_clock.m_date.setDate( 2000, 1, 1 );
        m_clock.m_time.setHMS( 0, 0, 0 );
        m_data[0] = 1<<7; // Disabled
    }
    m_data[7] = 0x03;
}

void DS1307::stamp()
{
    TwiModule::setMode( TWI_SLAVE );
}

void DS1307::readByte()               // Write to RAM
{
    if( m_phase == 0 ){
        m_phase = 1;
        m_addrPtr = m_rxReg;
    }else{
        m_data[m_addrPtr] = m_rxReg;
        if( m_addrPtr < 8 ){
            if     ( m_addrPtr < 3 ) updtTime();
            else if( m_addrPtr < 7 ) updtDate();
            else                     updtCtrl();
        }
        m_addrPtr++;
        if( m_addrPtr == 64 ) m_addrPtr = 0;
    }
    TwiModule::readByte();
}

void DS1307::writeByte()               // Read from RAM
{
    if( m_addrPtr < 7 ){
        switch( m_addrPtr ){
            case 0: {
                uint8_t disabled = m_data[0] & (1<<7);
                m_txReg = decToBcd( m_clock.m_time.second() ) | disabled; break;
            }break;
            case 1: m_txReg = decToBcd( m_clock.m_time.minute() ); break;
            case 2: {
                int hour = m_clock.m_time.hour();
                uint8_t ampm = m_data[2] & (1<<6); // 12/24 hour
                if( ampm && hour > 12 ) hour -= 12; // 12 hour PM
                m_txReg = ampm | decToBcd( hour );
            }break;
            case 3: m_txReg = decToBcd( m_clock.m_date.dayOfWeek() ); break;
            case 4: m_txReg = decToBcd( m_clock.m_date.day() ); break;
            case 5: m_txReg = decToBcd( m_clock.m_date.month() ); break;
            case 6: m_txReg = decToBcd( m_clock.m_date.year() - 2000 ); break;
        }
    }
    else m_txReg = m_data[m_addrPtr];
    m_addrPtr++;
    if( m_addrPtr == 64 ) m_addrPtr = 0;

    TwiModule::writeByte();
}

void DS1307::I2Cstop()
{
    m_phase = 0;
    TwiModule::I2Cstop();
}

void DS1307::updtTime()
{
    switch( m_addrPtr ){
        case 0: {
            m_clock.disable( m_data[0] & 1<<7 );
            m_clock.m_time.setHMS( m_clock.m_time.hour(), m_clock.m_time.minute(), bcdToDec( m_rxReg & ~(1<<7) ) ); break;
        }
        case 1: m_clock.m_time.setHMS( m_clock.m_time.hour(), bcdToDec( m_rxReg ), m_clock.m_time.second() ); break;
        case 2: {
            int hour;
            if( m_rxReg >= 0x40 ) // 12 hour
            {
                hour = bcdToDec( m_rxReg & 0b00011111 );
                if( m_rxReg & (1<<5) ) hour += 12;       // PM
            }
            else hour = bcdToDec( m_rxReg & 0b00111111 ); // 24 hout
            m_clock.m_time.setHMS( hour, m_clock.m_time.minute(), m_clock.m_time.second() );
        }
    }
}

void DS1307::updtDate()
{
    switch( m_addrPtr ){
        case 4: m_clock.m_date.setDate( m_clock.m_date.year()   , m_clock.m_date.month(), bcdToDec( m_rxReg )  ); break;
        case 5: m_clock.m_date.setDate( m_clock.m_date.year()   , bcdToDec( m_rxReg )   , m_clock.m_date.day() ); break;
        case 6: m_clock.m_date.setDate( 2000+bcdToDec( m_rxReg ), m_clock.m_date.month(), m_clock.m_date.day() );
    }
}

void DS1307::updtCtrl()
{
    char RS = m_data[7] & 0b00000011;
    uint64_t freq = 0;  // Hz
    switch( RS ){
        case 0: freq = 1;     break;
        case 1: freq = 4096;  break;
        case 2: freq = 8192;  break;
        case 3: freq = 32768; break;
    }
    m_clock.setFreq( freq );
    m_clock.enableOut( m_data[7] & (1<<4) );
    m_clock.setDisOut( m_data[7] & (1<<7) );
}

char DS1307::decToBcd(char val) {
    return( (val/10*16) + (val%10) ); }
char DS1307::bcdToDec(char val) {
    return( (val/16*10) + (val%16) ); }

void DS1307::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area, 2, 2 );
    p->drawPixmap( QRect(-28,-20, 56, 40 ), QPixmap( m_background ));
}
