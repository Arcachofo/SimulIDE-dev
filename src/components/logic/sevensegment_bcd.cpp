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
        "7-Seg BCD", /// FIXME: only alphanumeric in Component type
        SevenSegmentBCD::construct );
}

SevenSegmentBCD::SevenSegmentBCD( QString type, QString id )
               : BcdBase( type, id )
{
    m_graphical = true;

    m_width  = 4;
    m_height = 6;
    m_color  = Qt::black;

    QStringList pinList;

    pinList // Inputs:
            << "ID041"
            << "ID032"
            << "ID024"
            << "ID018"
            << "IU04E"
            << "IU01."
            ;
    init( pinList );
    for( uint i=0; i<m_inPin.size(); ++i )
    {
        m_inPin[i]->setX( m_inPin[i]->x()-4);
        m_inPin[i]->setSpace( 1 );
        m_inPin[i]->setFontSize( 4 );
        m_inPin[i]->setLabelColor( QColor( 250, 250, 200 ) );
    }

    m_showEnablePin = false;
    m_enablePin = m_inPin[4];
    m_enablePin->setInverted( true );
    m_enablePin->setVisible( false );

    m_showDotPin = false;
    m_dotPin = m_inPin[5];
    m_dotPin->setVisible( false );

    setLabelPos(-16,-40, 0);

    addPropGroup( { tr("Main"), {
        new BoolProp<SevenSegmentBCD>("Show_Point_Pin", tr("Show Point Pin"),"", this
                              , &SevenSegmentBCD::isShowDotPin, &SevenSegmentBCD::setShowDotPin, propNoCopy ),

        new BoolProp<SevenSegmentBCD>("Show_Enable_Pin", tr("Show Enable Pin"),"", this
                              , &SevenSegmentBCD::isShowEnablePin, &SevenSegmentBCD::setShowEnablePin, propNoCopy )
    },groupNoCopy} );

    Simulator::self()->addToUpdateList( this );

    m_digit = 0;
}
SevenSegmentBCD::~SevenSegmentBCD(){}

void SevenSegmentBCD::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    if( !Simulator::self()->isRunning() ) m_digit = 0;
    else if( !m_linkedTo ){
        if( m_enablePin->getInpState() ){
            BcdBase::voltChanged();
            if( m_dotPin->getInpState() ) m_digit |= 0x80;
        }
        else m_digit = 0;
    }
    update();
}

void SevenSegmentBCD::voltChanged() { m_changed = true; }

void SevenSegmentBCD::setShowEnablePin( bool show )
{
    if( m_showEnablePin == show ) return;
    m_showEnablePin = show;

    if( !show ) m_enablePin->removeConnector();
    m_enablePin->setVisible( show );
}

void SevenSegmentBCD::setShowDotPin( bool show )
{
    if( m_showDotPin == show) return;
    m_showDotPin = show;

    if( !show ) m_dotPin->removeConnector();
    m_dotPin->setVisible( show );
}

bool SevenSegmentBCD::setLinkedTo( Linker* li )
{
    bool linked = Component::setLinkedTo( li );
    if( li && linked )
        for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->removeConnector();

    setHidden( (li && linked), false, false );

    return linked;
}

void SevenSegmentBCD::setLinkedValue( double v, int i )
{
    int vInt = v;
    vInt &= 0xFF;
    if( i == 0 ) m_digit = m_values[vInt]; // Display value
    else         m_digit = vInt;           // 1 bit for each segment
    m_changed = true;
}

void SevenSegmentBCD::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );
    p->drawRect( m_area );

    const int mg =  6; // Margin around number
    const int ds =  1; // "Slope"
    const int tk =  4; // Line thickness
    const int x1 =  m_area.x()+mg;
    const int x2 = -m_area.x()-mg;
    const int y1 =  m_area.y()+mg;
    const int y2 = -m_area.y()-mg;

    QPen pen;
    pen.setWidth( tk );
    QColor color = LedBase::getColor( (LedBase::ledColor_t)0, 255 );
    pen.setColor( color );
    pen.setCapStyle( Qt::RoundCap );
    p->setPen( pen );

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
    Component::paintSelected( p );
}
