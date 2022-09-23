/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "max72xx_matrix.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"

#include "stringprop.h"
#include "intprop.h"

Component* Max72xx_matrix::construct( QObject* parent, QString type, QString id )
{ return new Max72xx_matrix( parent, type, id ); }

LibraryItem* Max72xx_matrix::libraryItem()
{
    return new LibraryItem(
        tr( "Max72xx_matrix" ),
        tr( "Leds" ),
        "max72xx.png",
        "Max72xx_matrix",
        Max72xx_matrix::construct);
}

Max72xx_matrix::Max72xx_matrix( QObject* parent, QString type, QString id )
              : LogicComponent( parent, type, id )
{
    m_graphical = true;
    m_numDisplays = 4;
    m_area = QRectF(-36, -44, 4+64*m_numDisplays+4, 88 );

    m_enumUids = QStringList()
        << "Yellow"
        << "Red"
        << "Green"
        << "Blue"
        << "Orange"
        << "Purple"
        << "White";

    m_enumNames = QStringList()
        << QObject::tr("Yellow")
        << QObject::tr("Red")
        << QObject::tr("Green")
        << QObject::tr("Blue")
        << QObject::tr("Orange")
        << QObject::tr("Purple")
        << QObject::tr("White");

    m_colors[0] = QColor( 255, 255,   0 ); // Yellow
    m_colors[1] = QColor( 255, 140, 120 ); // Red
    m_colors[2] = QColor(   0, 255, 100 ); // Green
    m_colors[3] = QColor( 150, 150, 255 ); // Blue
    m_colors[4] = QColor( 255, 200,  25 ); // Orange
    m_colors[5] = QColor( 255,  75, 255 ); // Purple
    m_colors[6] = QColor( 255, 255, 255 ); // White
    m_ledColor = 0;

    m_pinCS  = new Pin( 270, QPoint(-12, 52), id+"PinCS", 0, this );
    m_pinDin = new Pin( 270, QPoint(-20, 52), id+"PinDin", 0, this );
    m_pinSck = new IoPin( 270, QPoint(-28, 52), id+"PinSck", 0, this, input );

    m_pinCS->setLabelText(  " CS" );
    m_pinDin->setLabelText( " DIN" );
    m_pinSck->setLabelText( " SCK" );

    m_pin.resize( 3 );
    m_pin[0] = m_pinCS;
    m_pin[1] = m_pinDin;
    m_pin[2] = m_pinSck;

    eClockedDevice::setClockPin( m_pinSck );

    Simulator::self()->addToUpdateList( this );

    setLabelPos(-32, -58, 0);
    setShowId( true );

    initialize();

    addPropGroup( { tr("Main"), {
new StringProp<Max72xx_matrix>( "Color"   , tr("Color")   ,""       , this, &Max72xx_matrix::colorStr,    &Max72xx_matrix::setColorStr, "enum" ),
new IntProp   <Max72xx_matrix>( "NumDisplays", tr("Size"),"_8x8 Led", this, &Max72xx_matrix::numDisplays, &Max72xx_matrix::setNumDisplays, "uint" ),
    }} );
}
Max72xx_matrix::~Max72xx_matrix(){}

void Max72xx_matrix::stamp()
{
    m_pinSck->changeCallBack( this ); // Register for Sck changes callback
    m_pinCS->changeCallBack( this );  // Register for CS changes callback
}

void Max72xx_matrix::initialize()
{
    for( int i=0; i<11; i++)
    {
        for( int j=0; j<8; j++ ) m_ram[i][j] = 0;
        m_intensity[i] = 0;
    }
    m_decodemode = 0;
    m_scanlimit = 0;
    m_shutdown = true;
    m_test = false;

    m_rxReg = 0;
    m_inBit = 0;
    m_inDisplay = 0;

    updateStep();
}

void Max72xx_matrix::voltChanged()
{
    updateClock();

    if( m_pinCS->getVoltage()>1.6 )            // CS high: not selected
    {
        m_rxReg = 0;
        m_inBit  = 0;
        m_inDisplay = 0;
        return;
    }
    if( m_clkState != Clock_Rising ) return;

    m_rxReg &= ~1;
    if( m_pinDin->getVoltage()>1.6 ) m_rxReg |= 1;

    if( m_inBit == 15 )
    {
        proccessCommand();
        m_inBit = 0;
        m_inDisplay++;
    }else{
        m_rxReg <<= 1;
        m_inBit++;
}   }

void Max72xx_matrix::updateStep() { update(); }

void Max72xx_matrix::proccessCommand()
{
    if ( m_inDisplay >= 11 ) return;

    int addr = (m_rxReg>>8) & 0x0F;
    switch( addr )
    {
        case 0: break; // No-Op
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:  // Digits 0 to 7
            m_ram[m_inDisplay][addr-1] = m_rxReg & 0xFF;
            break;
        case 9:  // Decode mode (only no-decode 0 mode supported)
            m_decodemode = m_rxReg & 0xFF;
            break;
        case 10: // Intensity (0-15)
            m_intensity[m_inDisplay] = m_rxReg & 0x0F;
            break;
        case 11: // Scan limit (0-7)
            m_scanlimit = m_rxReg & 0x07;
            break;
        case 12: // Shutdown
            m_shutdown = !(m_rxReg & 0x01);
            break;
        case 15: // Display test
            m_test = m_rxReg & 0x01;
            break;
}   }

void Max72xx_matrix::setNumDisplays( int displays )
{
    if( displays == m_numDisplays ) return;
    if( displays < 1 ) displays = 1;
    if( displays > 11 ) displays = 11;
    m_numDisplays = displays;

    hide();
    m_area = QRectF(-36, -44, 4+64*m_numDisplays+4, 88 );
    show();
}

void Max72xx_matrix::setColorStr( QString color )
{
    m_ledColor = getEnumIndex( color );
    if( m_showVal && (m_showProperty == "Color") )
        setValLabelText( m_enumNames.at( m_ledColor ) );
}

void Max72xx_matrix::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->setRenderHint( QPainter::Antialiasing );

    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );

    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area, 2, 2 );

    for( int display=0; display<m_numDisplays; display++ )
    {
        pen.setColor( QColor(Qt::black) );
        p->setPen( pen );
        p->setBrush( QColor(Qt::black) );
        p->drawRoundedRect( 64*display-32, -40, 64, 64, 2, 2 );

        QColor color = m_colors[m_ledColor];
        int factor = 100 + (15-m_intensity[display])*3;
        color = color.darker(factor);

        for( int row=0; row<8; row++ )
        {
            int x = 64*display-32+1;
            int y = -40+row*8+1;
            for( int col=0; col<8; col++)
            {
                int bit = m_ram[display][row] & (0x80>>col);
                if( !m_shutdown && (m_test || ((m_decodemode == 0) && (row <= m_scanlimit) && bit)) )
                {
                    p->setBrush( color );
                    pen.setColor( color );
                }else{
                    p->setBrush( QColor(50, 50, 50) );
                    pen.setColor( QColor(50, 50, 50) );
                }
                p->setPen( pen );
                p->drawEllipse( x, y, 6, 6 );
                x += 8;
}   }   }   }
