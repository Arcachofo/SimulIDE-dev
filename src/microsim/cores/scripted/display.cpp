/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <math.h>

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

void Display::drawLine( int x0, int y0, int x1, int y1, int color )
{
    int dx = fabs( x1-x0 );
    int dy = fabs( y1-y0 );

    int stepX = x0 < x1 ? 1 : -1;
    int stepY = y0 < y1 ? 1 : -1;

    int err = dx - dy;

    while( true )
    {
        setPixel( x0, y0, color );

        if( x0 == x1 && y0 == y1 ) break;

        int e2 = 2 * err;

        if( e2 > -dy ){
            err -= dy;
            x0 += stepX;
        }
        if( e2 < dx ){
            err += dx;
            y0 += stepY;
        }
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

