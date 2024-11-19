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

#define tr(str) simulideTr("Ds18b20",str)

Component* Ds18b20::construct( QString type, QString id )
{ return new Ds18b20( type, id ); }

LibraryItem* Ds18b20::libraryItem()
{
    return new LibraryItem(
        "DS18B20",
        "Sensors",
        "ic2_comp.png",
        "DS18B20",
        Ds18b20::construct );
}

Ds18b20::Ds18b20( QString type, QString id )
       : Component( type, id )
       , OneWire( id )
{
    m_graphical = true;
    m_area = QRect(-28,-16, 56, 32 );

    srand( time(0) );

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
    u_button->setCursor( Qt::PointingHandCursor );

    QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( u_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( -7, 4 ) );

    QPushButton* d_button = new QPushButton();
    d_button->setMaximumSize( 9, 9 );
    d_button->setGeometry(-5,-5, 9, 9 );
    d_button->setCheckable( false );
    d_button->setIcon( QIcon(":/giu.png") );
    d_button->setCursor( Qt::PointingHandCursor );

    proxy = Circuit::self()->addWidget( d_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( 9, 4 ) );

    QObject::connect( u_button, &QPushButton::pressed, [=](){ upbuttonclicked(); } );
    QObject::connect( d_button, &QPushButton::pressed, [=](){ downbuttonclicked(); } );

    m_font.setFamily("Ubuntu Mono");
    m_font.setPixelSize( 10 );
    m_font.setBold( true );
    m_font.setLetterSpacing( QFont::PercentageSpacing, 100 );
    setLabelPos(-24,-28 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new StrProp <Ds18b20>("ROM","ROM", ""
                             , this, &Ds18b20::getROM, &Ds18b20::setROM, propNoCopy ),

        new DoubProp<Ds18b20>("Temp", tr("Temperature"), "°C"
                             , this, &Ds18b20::temp, &Ds18b20::setTemp ),

        new DoubProp<Ds18b20>("TempInc", tr("Temp. increment"), "°C"
                             , this, &Ds18b20::tempInc, &Ds18b20::setTempInc ),
    },0} );
}
Ds18b20::~Ds18b20(){}

void Ds18b20::updateStep()
{
    setTemp( m_temp );
    Simulator::self()->remFromUpdateList( this );
}

void Ds18b20::readByte()
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

void Ds18b20::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );

    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( QRect(-26,-14, 52, 15 ),2,2 );

    p->setFont( m_font );
    p->setPen( QColor(0, 0, 0) );
    p->drawText(-23, -3, QString::number( m_temp )+"°C" );

    Component::paintSelected( p );
}
