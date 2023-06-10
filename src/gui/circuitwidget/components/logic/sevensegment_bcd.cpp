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

Component* SevenSegmentBCD::construct( QObject* parent, QString type, QString id )
{ return new SevenSegmentBCD( parent, type, id ); }

LibraryItem* SevenSegmentBCD::libraryItem()
{
    return new LibraryItem(
        tr( "7 Seg BCD" ),
        "Other Logic",
        "7segbcd.png",
        "7-Seg BCD",
        SevenSegmentBCD::construct );
}

SevenSegmentBCD::SevenSegmentBCD( QObject* parent, QString type, QString id )
               : BcdBase( parent, type, id )
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
    setLabelPos(-16,-40, 0);
        
    Simulator::self()->addToUpdateList( this );
    
    initialize();
}
SevenSegmentBCD::~SevenSegmentBCD(){}

void SevenSegmentBCD::updateStep()
{
    if( !m_changed ) return;

    update();
    m_changed = false;
}

void SevenSegmentBCD::setLinked( bool l )
{
    Component::setLinked( l );
    if( l )
        for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->removeConnector();

    setHidden( l, false, false );
}

void SevenSegmentBCD::setLinkedValue( int v, int i )
{
    v &= 0xFF;
    if( i == 0 ) m_digit = m_values[v]; // Display value
    else         m_digit = v & 0xFF;    // 1 bit for each segment
    m_changed = true;
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
