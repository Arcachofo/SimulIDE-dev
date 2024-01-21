/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <math.h>

#include "ks0108.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"

#include "boolprop.h"

#define tr(str) simulideTr("Ks0108",str)

Component* Ks0108::construct( QString type, QString id )
{ return new Ks0108( type, id ); }

LibraryItem* Ks0108::libraryItem()
{
    return new LibraryItem(
        "KS0108",
        "Displays",
        "ks0108.png",
        "Ks0108",
        Ks0108::construct );
}

Ks0108::Ks0108( QString type, QString id )
      : Component( type, id )
      , eElement( (id+"-eElement") )
      , m_pinRst( 270, QPoint(-56, 56), id+"-PinRst" , 0, this )
      , m_pinCs2( 270, QPoint(-48, 56), id+"-PinCs2" , 0, this )
      , m_pinCs1( 270, QPoint(-40, 56), id+"-PinCs1" , 0, this )
      , m_pinEn ( 270, QPoint( 32, 56), id+"-PinEn"  , 0, this )
      , m_pinRW ( 270, QPoint( 40, 56), id+"-PinRW"  , 0, this )
      , m_pinDC ( 270, QPoint( 48, 56), id+"-PinDC"  , 0, this )
{
    m_area = QRectF( -74, -52, 148, 100 );
    m_graphical = true;
    m_csActLow = false;
    
    m_pinRst.setLabelText( "RST" );
    m_pinCs1.setLabelText( "CS1" );
    m_pinCs2.setLabelText( "CS2" );
    m_pinDC.setLabelText(  "RS" );
    m_pinRW.setLabelText(  "RW" );
    m_pinEn.setLabelText(  "En" );
    
    m_dataPin.resize( 8 );
    m_pin.resize( 14 );
    
    int pinY = 56;
    for( int i=0; i<8; i++ )
    {
        QString pinId = id+"-dataPin"+QString::number(i);
        m_dataPin[i] = new IoPin( 270, QPoint(-32+(7-i)*8, pinY), pinId , 0, this, input );
        m_dataPin[i]->setLabelText( "D"+QString::number(i) );
        m_dataPin[i]->setOutHighV( 5 );
        m_pin[i] = m_dataPin[i];
    }
    m_pin[8]  = &m_pinRst;
    m_pin[9]  = &m_pinCs2;
    m_pin[10] = &m_pinCs1;
    m_pin[11] = &m_pinEn;
    m_pin[12] = &m_pinRW;
    m_pin[13] = &m_pinDC;

    Simulator::self()->addToUpdateList( this );
    
    setLabelPos( -32,-68, 0);
    setShowId( true );
    
    Ks0108::initialize();

    addPropGroup( { tr("Main"), {
new BoolProp<Ks0108>("CS_Active_Low", tr("CS Active Low"), ""
                    , this, &Ks0108::csActLow, &Ks0108::setCsActLow )
    }, groupNoCopy} );
}
Ks0108::~Ks0108()
{
}

void Ks0108::initialize()
{
    clearDDRAM();
    reset() ;
    updateStep();
}

void Ks0108::stamp()
{
    m_pinEn.changeCallBack( this ); // Register for Scl changes callback
    m_pinRst.changeCallBack( this ); // Register for Rst changes callback
}

void Ks0108::updateStep()
{
    update();
}

void Ks0108::voltChanged()                 // Called when En Pin changes 
{
    if( m_pinRst.getVoltage()<2.5 ) reset();            // Reset Pin is Low
    else                         m_reset = false;
    
    bool Write = ( m_pinRW.getVoltage()<2.5 );             // Read or Write
    if( m_Write != Write )               // Set Read or Write Impedances
    {
        m_Write = Write;
        for( int i=0; i<8; i++ ) 
        {
            if( Write ) m_dataPin[i]->setPinMode( input );
            else        m_dataPin[i]->setPinMode( output );
    }   }
    bool Scl = (m_pinEn.getVoltage()>2.5);
    
    if    ( Scl && !m_lastScl ){           // This is a clock Rising Edge
        m_lastScl = true;  
        if( Write ) return;                  // Only Read in Rising Edge
    }
    else if( !Scl && m_lastScl ){         // This is a clock Falling edge
        m_lastScl = false;  
        if( !Write ) return;               // Only Write in Falling Edge
    }else{
        m_lastScl = Scl;
        return;
    }
    m_input = 0;
    if( Write ){
        for( int pin=0; pin<8; pin++ )                     // Read input
        {
            if( m_dataPin[pin]->getVoltage()>2.5 )  m_input += pow( 2, pin );
    }   }
    m_Cs1 =  (m_pinCs1.getVoltage()>2.5);               // Half 1 selected?
    m_Cs2 =  (m_pinCs2.getVoltage()>2.5);               // Half 2 selected?
    if( !m_Cs1 & !m_Cs2 ) m_Cs2 = true;
    
    if( m_csActLow ) {
        m_Cs1 = !m_Cs1;
        m_Cs2 = !m_Cs2;
    }
    
    if( (m_pinDC.getVoltage()>2.5)               )                  // Data
    {
        if( m_reset ) return;            // Can't write data while Reset
        if( Write ) writeData( m_input );                  // Write Data
        else        ReadData();                             // Read Data
    }
    else{                                                     // Command
        if( Write ) proccessCommand( m_input );          // Write Command
        else        ReadStatus();                          // Read Status
}   }

void Ks0108::ReadData()
{
    int data = 0;
    if( m_Cs1 ) data = m_aDispRam[m_addrX1][m_addrY1];
    if( m_Cs2 ) data = m_aDispRam[m_addrX2][m_addrY2+64];

    for( int i=0; i<8; i++ )
    {
        m_dataPin[i]->scheduleState( (data & 1)==1, 0 );
        data >>= 1;
}   }

void Ks0108::ReadStatus()
{
    for( int i=0; i<8; i++ ) 
    {
        bool out = false;
        if     ( i == 4 ) out = m_reset;
        else if( i == 5 ) out = !m_dispOn;
        m_dataPin[i]->scheduleState( out, 0 );
}   }

void Ks0108::writeData( int data )
{
    if( m_Cs1 ) m_aDispRam[m_addrX1][m_addrY1]    = data;  // Write Half 1
    if( m_Cs2 ) m_aDispRam[m_addrX2][m_addrY2+64] = data;  // Write Half 2
    incrementPointer();
}

void Ks0108::proccessCommand( int command )
{
    //qDebug() << "Ks0108::proccessCommand: " << command;
    if( command<62 )  {                           return; }             // Not Valid
    if( command<64 )  { dispOn( command & 1 )   ; return; } //0011111.  // Display On/Off internal status &Ram not affected
    if( command<128 ) { setYaddr( command & 63 ); return; } //01......  // Set Y address
    if( command<184 ) {                           return; }             // Not Valid
    if( command<192 ) { setXaddr( command & 7 );  return; } //10111...  // Set X address     
    else              { startLin( command & 63 ); return; } //11......  // Set Display Start Line
}
void Ks0108::dispOn( int state ) { m_dispOn = (state > 0); }

void Ks0108::setYaddr( int addr )
{
    if( m_Cs1 ) m_addrY1  = addr ;
    if( m_Cs2 ) m_addrY2  = addr ;
}

void Ks0108::setXaddr( int addr )
{
    if( m_Cs1 ) m_addrX1  = addr ;
    if( m_Cs2 ) m_addrX2  = addr ;
}

void Ks0108::clearDDRAM() 
{
    for(int row=0;row<8;row++) 
        for( int col=0;col<128;col++ ) 
            m_aDispRam[row][col] = 0;
}

void Ks0108::incrementPointer() 
{
    if( m_Cs1 ){
        m_addrY1++;
        if( m_addrY1 > 63 )m_addrY1 = 0;
    }
    if( m_Cs2 ){
        m_addrY2++;
        if( m_addrY2 > 63 )m_addrY2 = 0;
}   }

void Ks0108::reset() 
{
    m_addrX1  = 0;
    m_addrY1  = 0;
    m_addrX2  = 0;
    m_addrY2  = 0;
    m_startLin = 0;
    m_dispOn = false;
    m_reset  = true;
}

void Ks0108::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );
    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( -70, -48, 140, 76, 8, 8 );

    if( !m_dispOn ) p->fillRect(-64,-42, 128, 64, QColor(200,215,180) );
    else{
        QImage img( 128*3, 64*3, QImage::Format_RGB32 );
        QPainter painter;
        painter.begin( &img );
        painter.fillRect( 0, 0, 128*3, 64*3, QColor(200,215,180) );

        for(int row=0;row<8;row++){
            for( int col=0; col<128; col++ )
            {
                int x = col*3;
                char abyte = m_aDispRam[row][col];

                for( int bit=0; bit<8; bit++ )
                {
                    if( abyte & 1 ){
                        int y = row*8+bit;
                        painter.fillRect( x, y*3, 3, 3, Qt::black );
                    }
                    abyte >>= 1;
        }   }   }

        painter.end();
        p->drawImage(QRectF(-64,-42, 128, 64), img );
    }

    Component::paintSelected( p );
}
