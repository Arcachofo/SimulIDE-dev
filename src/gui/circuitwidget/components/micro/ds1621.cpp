/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QPushButton>
#include <QGraphicsProxyWidget>

#include <math.h>

#include "ds1621.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "iopin.h"
#include "simulator.h"

#include "doubleprop.h"

Component *DS1621::construct(QObject *parent, QString type, QString id) {
  return new DS1621( parent, type, id );
}

LibraryItem *DS1621::libraryItem()
{
    return new LibraryItem(
        tr("DS1621"),
        "Sensors",
        "ic_comp.png",
        "DS1621",
        DS1621::construct);
}

DS1621::DS1621( QObject* parent, QString type, QString id )
      : IoComponent( parent, type, id )
      , TwiModule( id )
{
    m_width  = 7;
    m_height = 4;

    init({           // Inputs:
      "IL01SDA", // type: Input, side: Left, pos: 01, label: "SDA"
      "IL02SCL",
      "IR01A0",
      "IR02A1",
      "IR03A2",
      // Outputs:
      "OL03Tout"
     });

    m_inPin[0]->setPinMode( openCo );
    TwiModule::setSdaPin( m_inPin[0] );

    m_inPin[1]->setPinMode( openCo );
    TwiModule::setSclPin( m_inPin[1] );

    for( IoPin* pin : m_inPin )  pin->setLabelColor( QColor( 250, 250, 200 ) );
    for( IoPin* pin : m_outPin ) pin->setLabelColor( QColor( 250, 250, 200 ) );

    QPushButton* u_button = new QPushButton();
    u_button->setMaximumSize( 9, 9 );
    u_button->setGeometry(-5,-5, 9, 9);
    u_button->setCheckable( false );
    u_button->setIcon(QIcon(":/su.png"));

    QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( u_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( -7, 4 ) );

    QPushButton* d_button = new QPushButton();
    d_button->setMaximumSize( 9, 9 );
    d_button->setGeometry(-5,-5, 9, 9);
    d_button->setCheckable( false );
    d_button->setIcon(QIcon(":/giu.png"));

    proxy = Circuit::self()->addWidget( d_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( 9, 4 ) );

    connect( u_button, &QPushButton::pressed,
             this,     &DS1621::upbuttonclicked );

    connect( d_button, &QPushButton::pressed,
             this,     &DS1621::downbuttonclicked );

    m_font.setFamily("Ubuntu Mono");
#ifdef _WIN32
    m_font.setStretch( 99 );
#else
    m_font.setStretch( 93 );
#endif
    m_font.setPixelSize( 10 );
    m_font.setBold( true );
    m_font.setLetterSpacing( QFont::PercentageSpacing, 100 );
    setLabelPos(-24,-28 );

    m_temp = 22.5;
    m_tempInc = 0.5;
    m_changed = true;

    //IoComponent::initState();
    m_cCode = 0b01001000; // 0x98 >> 1  : I2C Addr

    addPropGroup( { tr("Main"), {
new DoubProp<DS1621>("Temp"   , tr("Temperature")    ,"°C", this, &DS1621::temp   , &DS1621::setTemp ),
new DoubProp<DS1621>("TempInc", tr("Temp. increment"),"°C", this, &DS1621::tempInc, &DS1621::setTempInc ),
    },0} );
}

DS1621::~DS1621() {}

void DS1621::stamp() // Called at Simulation Start
{
    m_command = 0;
    m_changed = true;
    m_tempCount = 0;
    m_tempSlope = 0;
    m_writeByte = -1;
    m_config = 0;
    m_convert = false;
    m_oneShot = false;
    m_outPol  = false;

    m_Th = 0;
    m_Tl = 0;

    TwiModule::setMode( TWI_SLAVE );

    for( int i=2; i<5; i++ ) m_inPin[i]->changeCallBack( this );
}

void DS1621::runEvent() // Conversion done 750  ms
{
    if( !m_convert ) return;

    m_config |= 1<<7; // Set DONE bit

    if( m_changed ) doConvert();
    if( !m_oneShot ) Simulator::self()->addEvent( 750*1e9, this ); // Continuous
}

void DS1621::voltChanged()             // Some Pin Changed State, Manage it
{
    m_address = m_cCode;
    if( m_inPin[2]->getInpState() ) m_address += 1;
    if( m_inPin[3]->getInpState() ) m_address += 2;
    if( m_inPin[4]->getInpState() ) m_address += 4;

    TwiModule::voltChanged();        // Run I2C Engine
}

void DS1621::readByte() // read from I2C
{
    bool convert = m_convert;

    if( m_command == 0x00 )      // Master sending Command
    {
        m_writeByte = -1;
        if     ( m_rxReg == 0x22 ) convert = false;
        else if( m_rxReg == 0xEE ) convert = true;
        else{
            m_command = m_rxReg;
            if     ( m_command == 0xA1 ) m_writeByte = 1;  // Access TH
            else if( m_command == 0xA2 ) m_writeByte = 1;  // Access TL
            else if( m_command == 0xA8 ) m_writeByte = 0;  // Read Counter
            else if( m_command == 0xA9 ) m_writeByte = 0;  // Read Slope
            else if( m_command == 0xAA ) m_writeByte = 1;  // Read Temp
            else if( m_command == 0xAC ) m_writeByte = 0;  // Access Config
        }
        if( convert != m_convert ){
            m_convert = convert;
            Simulator::self()->cancelEvents( this );

            if( convert ) // Start Conversion
            {
                m_config &= ~(1<<7); // Clear DONE bit
                Simulator::self()->addEvent( 750*1e9, this ); // 750 ms
            }
        }
    }else{                         // Master writting Data
        if( m_writeByte >= 0  ){
            if     ( m_command == 0xA1 ) // Write TH
            {
                m_ThReg[ m_writeByte ] = m_rxReg;
                m_Th = m_ThReg[1];
                double thH = m_ThReg[0] ? 0.5 : 0.0;
                if( m_Th < 0 ) thH = -thH;
                m_Th += thH;
            }
            else if( m_command == 0xA2 ) // Write TL
            {
                m_TlReg[ m_writeByte ] = m_rxReg;
                m_Tl = m_TlReg[1];
                double tlH = m_TlReg[0] ? 0.5 : 0.0;
                if( m_Tl < 0 ) tlH = -tlH;
                m_Tl += tlH;
            }
            else if( m_command == 0xAC ) // Write Config
            {
                m_config = m_rxReg;
                m_oneShot = m_config & 1;
                m_outPol  = m_config & 2;
            }
        }
        if( --m_writeByte < 0 ) m_command = 0;
    }
    TwiModule::readByte();
}

void DS1621::writeByte() // write to I2C
{
    m_txReg = 0;
    if( m_writeByte >= 0 ){
        if     ( m_command == 0xA1 ) m_txReg = m_ThReg[ m_writeByte ]; // Read TH
        else if( m_command == 0xA2 ) m_txReg = m_TlReg[ m_writeByte ]; // Read TL
        else if( m_command == 0xA8 ) m_txReg = m_tempCount;            // Read Counter
        else if( m_command == 0xA9 ) m_txReg = m_tempSlope;            // Read Slope
        else if( m_command == 0xAA ) m_txReg = m_tempReg[m_writeByte]; // Read Temp
        else if( m_command == 0xAC ) m_txReg = m_config;               // Read Config
    }
    if( --m_writeByte < 0 ) m_command = 0;

    TwiModule::writeByte();
}

void DS1621::doConvert()
{
    m_changed = false;

    float temp_abs = fabs( m_temp ); // make comptutations with absolute value
    m_tempReg[1] = temp_abs;
    float temp_frac = temp_abs - m_tempReg[1];
    if( temp_frac >= 0.75) m_tempReg[1] += 1;

    m_tempReg[0] = 0;
    if( (temp_frac >= 0.25) && (temp_frac < 0.75)) m_tempReg[0] = 0x80;

    m_tempSlope = 16;          // compute high resolution
    m_tempCount = m_tempSlope * (0.75 + m_tempReg[1] - temp_abs);

    if( m_temp < 0.0) m_tempReg[1] = -m_tempReg[1]; // take sign into account

    if( m_temp > m_Th ){
        m_config |= 1<<6;  // Set THF bit
        m_outPin[0]->setOutState( m_outPol );
    }
    else if( m_temp < m_Tl ){
        m_config |= 1<<5;  // Set TLF bit
        m_outPin[0]->setOutState( !m_outPol );
    }
}

void DS1621::upbuttonclicked()
{
    m_temp += m_tempInc;
    if( m_temp > 125 )  m_temp = 125;
    m_changed = true;
    update();
}

void DS1621::downbuttonclicked()
{
    m_temp -= m_tempInc;
    if( m_temp < -55 ) m_temp = -55;
    m_changed = true;
    update();
}

void DS1621::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );

    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( QRect(-11,-14, 33, 15 ),2,2 );

    p->setFont( m_font );
    p->setPen( QColor(0, 0, 0) );
    //p->drawText(-9, -3, "°C" );
    p->drawText(-9, -3, QString::number( m_temp )+"°C" );
}
