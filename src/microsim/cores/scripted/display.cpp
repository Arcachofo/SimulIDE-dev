/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "display.h"
#include "simulator.h"

Display::Display( uint w, uint h, QString name, QWidget* parent )
       : QWidget( parent )
       , Updatable()
       , eElement( name )
{
    m_width  = w;
    m_height = h;
    m_scale  = 1;

    m_background= 0;
    m_changed = false;
    m_embed   = true;

    updtImageSize();
}
Display::~Display(){}


void Display::initialize()
{
    //if( m_clear ) m_scriptCpu->callFunction( m_clear );
    m_x = 0;
    m_y = 0;
}

void Display::updateStep()
{
    if( m_changed )
    {
        m_changed = false;
        updtImageSize();
    }
    update();
}

void Display::setWidth( uint w )
{
    if( m_width == w || w < 1 ) return;
    m_width = w;
    m_changed = true;
}

void Display::setHeight( uint h )
{
    if( m_height == h || h < 1 ) return;
    m_height = h;
    m_changed = true;
}

void Display::setSize( uint w, uint h )
{
    if( w < 1 || h < 1 ) return;
    m_width  = w;
    m_height = h;
    m_changed = true;
}

void Display::setMonitorScale( double scale )
{
    if( scale <= 0 ) return;
    m_scale = scale;

    this->setFixedSize( m_width*scale, m_height*scale );

    Simulator::self()->addToUpdateList( this );

    show();
}

void Display::setBackground( int b )
{
    if( m_background == b ) return;
    m_background = b;
    updtImageSize();
}

void Display::clear()
{
    for( uint x=0; x<m_width; ++x )
        for( uint y=0; y<m_height; ++y )
            setPixel( x, y, m_background );
}

void Display::drawLine( uint x0, uint y0, uint x1, uint y1, int color )
{
    if( x0 > x1 ) {
        uint temp = x0;
        x0 = x1;
        x1 = temp;
    }
    if( y0 > y1 ) {
        uint temp = y0;
        y0 = y1;
        y1 = temp;
    }
    uint xDelta = x1-x0;
    uint yDelta = y1-y0;

    uint stepY = xDelta ? yDelta/xDelta : yDelta;
    uint endX  = xDelta ? x1-1 : x1;

    uint y;
    for( uint x=x0; x<=endX; ++x )
    {
        if( x >= m_width ) break;
        for( y=y0; y<=y0+stepY; ++y )
        {
            if( y >= m_height ) break;
            setPixel( x, y, color );
        }
        if( yDelta > 0 ) y0 = y;
    }
}

void Display::setPixel( uint x, uint y, int color )
{
    if( x >= m_width || y >= m_height ) return;
    m_data[x][y] = color;
}

void Display::updtImageSize()
{
    m_data.resize( 0, std::vector<int>(0) );
    m_data.resize( m_width, std::vector<int>(m_height, m_background) );
    this->setFixedSize( m_width*m_scale, m_height*m_scale );
}

void Display::paintEvent( QPaintEvent* )
{
    if( m_embed ) return;
    QPainter p(this);

    for( uint x=0; x<m_width; x++ )
        for( uint y=0; y<m_height; y++ )
            p.fillRect( QRectF( x*m_scale, y*m_scale, m_scale, m_scale ), QColor(m_data[x][y]) );
}

