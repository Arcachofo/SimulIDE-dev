/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <math.h>

#include "sevensegment_bcd.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"
#include "ledbase.h"

#include "boolprop.h"

#define tr(str) simulideTr("SevenSegmentBCD",str)

Component* SevenSegmentBCD::construct( QString type, QString id )
{ return new SevenSegmentBCD( type, id ); }

LibraryItem* SevenSegmentBCD::libraryItem()
{
    return new LibraryItem(
        tr("7 Seg BCD"),
        "Other Logic",
        "7segbcd.png",
        "7-Seg BCD", /// FIXME: only charaters in Component type
        SevenSegmentBCD::construct );
}

SevenSegmentBCD::SevenSegmentBCD( QString type, QString id )
               : BcdBase( type, id )
{
    m_graphical = true;

    m_width  = 4;
    m_height = 6;
    m_color = Qt::black;
    m_intensity = 255;

    QStringList pinList;

    pinList // Inputs:
            << "ID04  "
            << "ID03  "
            << "ID02  "
            << "ID01  "
            ;
    init( pinList );
    for( uint i=0; i<m_inPin.size(); ++i )
    {
        m_inPin[i]->setX( m_inPin[i]->x()-4);
        m_inPin[i]->setSpace( 1 );
        m_inPin[i]->setFontSize( 4 );
        m_inPin[i]->setLabelColor( QColor( 250, 250, 200 ) );
        m_inPin[i]->setLabelText( QString::number( pow(2,i) ) );
        m_inPin[i]->setLabelPos();
    }

    m_showEnablePin = false;
    m_enablePin = NULL;
    m_showDotPin = false;
    m_dotPin = NULL;

    setLabelPos(-16,-40, 0);

    addPropGroup( { tr("Main"), {
new BoolProp<SevenSegmentBCD>("Show_Point_Pin", tr("Show Point Pin"),"", this
                      , &SevenSegmentBCD::isShowDotPin, &SevenSegmentBCD::setShowDotPin, propNoCopy ),
new BoolProp<SevenSegmentBCD>("Show_Enable_Pin", tr("Show Enable Pin"),"", this
                      , &SevenSegmentBCD::isShowEnablePin, &SevenSegmentBCD::setShowEnablePin, propNoCopy )
    },groupNoCopy} );
        
    Simulator::self()->addToUpdateList( this );
    
    initialize();
}
SevenSegmentBCD::~SevenSegmentBCD(){}

void SevenSegmentBCD::stamp()
{
    BcdBase::stamp();
    if( m_enablePin ) m_enablePin->changeCallBack( this );
    if( m_dotPin ) m_dotPin->changeCallBack( this );
}

void SevenSegmentBCD::updateStep()
{
    if( !m_changed ) return;

    update();
    m_changed = false;
}

void SevenSegmentBCD::voltChanged()
{
    m_changed = true;
    bool enabled = true;
    if( m_showEnablePin ) enabled = m_enablePin->getVoltage() > 2.5;
    if( !enabled ) m_digit = 0;
    else {
        BcdBase::voltChanged();
        bool dotted = false;
        if( m_showDotPin ) dotted = m_dotPin->getVoltage() > 2.5;
        if( dotted ) m_digit |= 0x80;
    }
}

void SevenSegmentBCD::setShowEnablePin(bool v)
{
    if( m_showEnablePin == v) return;
    m_showEnablePin = v;
    if( v ) {
        m_enablePin = createPin("IU02", m_id+"-enable");
        m_enablePin->setInverted( true );
        m_enablePin->setX( m_inPin[0]->x() );
        m_enablePin->setFontSize( 4 );
        m_enablePin->setLabelColor( QColor( 250, 250, 200 ) );
        m_enablePin->setLabelText( "E" );
        m_enablePin->setLabelPos();
    } else {
        m_enablePin->removeConnector();
        delete m_enablePin;
        m_enablePin = NULL;
    }
}

void SevenSegmentBCD::setShowDotPin(bool v)
{
    if( m_showDotPin == v) return;
    m_showDotPin = v;
    if( v ) {
        m_dotPin = createPin("IU01", m_id+"-input-dot");
        m_dotPin->setX( m_inPin[3]->x() );
        m_dotPin->setFontSize( 4 );
        m_dotPin->setLabelColor( QColor( 250, 250, 200 ) );
        m_dotPin->setLabelText( "." );
        m_dotPin->setLabelPos();
    } else {
        m_dotPin->removeConnector();
        delete m_dotPin;
        m_dotPin = NULL;
    }
}

void SevenSegmentBCD::setLinked( bool l )
{
    Component::setLinked( l );
    if( l )
        for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->removeConnector();

    setHidden( l, false, false );
}

void SevenSegmentBCD::setLinkedValue( double v, int i )
{
    int vInt = v;
    vInt &= 0xFF;
    if( i == 0 ) m_digit = m_values[vInt]; // Display value
    else         m_digit = vInt;           // 1 bit for each segment
    m_changed = true;
}

std::vector<Pin*> SevenSegmentBCD::getPins()
{
    std::vector<Pin*> pins = BcdBase::getPins();
    if( m_dotPin  ) pins.emplace_back( m_dotPin );
    if( m_enablePin ) pins.emplace_back( m_enablePin );
    return pins;
}

void SevenSegmentBCD::remove()
{
    if( m_enablePin ) m_enablePin->removeConnector();
    if( m_dotPin ) m_dotPin->removeConnector();
    BcdBase::remove();
}

void SevenSegmentBCD::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );

    const int mg =  6;// Margin around number
    const int ds =  1; // "Slope"
    const int tk =  4; // Line thick
    const int x1 =  m_area.x()+mg;
    const int x2 = -m_area.x()-mg;
    const int y1 =  m_area.y()+mg;
    const int y2 = -m_area.y()-mg;

    QPen pen;
    pen.setWidth(tk);
    QColor color = LedBase::getColor( (LedBase::LedColor)0, m_intensity );
    pen.setColor( color );
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);

    if( m_digit & 1<<0 ) p->drawLine( x1+tk+ds, y1,    x2-tk+ds, y1    );
    if( m_digit & 1<<1 ) p->drawLine( x2+ds,    y1+tk, x2,      -tk    );
    if( m_digit & 1<<2 ) p->drawLine( x2,       tk,    x2-ds,    y2-tk );
    if( m_digit & 1<<3 ) p->drawLine( x2-tk-ds, y2,    x1+tk-ds, y2    );
    if( m_digit & 1<<4 ) p->drawLine( x1-ds,    y2-tk, x1,       tk    );
    if( m_digit & 1<<5 ) p->drawLine( x1,      -tk,    x1+ds,    y1+tk );
    if( m_digit & 1<<6 ) p->drawLine( x1+tk,    0,     x2-tk,    0     );
    if( m_digit & 1<<7 )
    {                               // Point
        p->setPen( Qt::NoPen );
        p->setBrush( QColor( 250, 250, 100) );
        p->drawPie( x2+ds, y2-ds, tk, tk, 0, 16*360 );
    };
}
