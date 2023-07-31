/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "display.h"
#include "simulator.h"

Display::Display( int w, int h, QString name, QWidget* parent )
       : QWidget( parent )
       , Updatable()
       , eElement( name )
       , m_image( w, h, QImage::Format_RGB888 )
{
    m_width  = w;
    m_height = h;
    m_name   = name;
    m_scale  = 0;

    m_background= 0;
    m_changed = false;

    m_image.fill( 0 );

    this->setFixedSize( m_width, m_height );
}
Display::~Display(){}


void Display::initialize()
{
    //if( m_clear ) m_scriptCpu->callFunction( m_clear );
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

void Display::setWidth( int w )
{
    if( m_width == w || w < 1 ) return;
    m_width = w;
    m_changed = true;
}

void Display::setHeight( int h )
{
    if( m_height == h || h < 1 ) return;
    m_height = h;
    m_changed = true;
}

void Display::setSize( int w, int h )
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
    m_image.fill( m_background );
}

void Display::setPixel( int x, int y, int color )
{
    if( x >= m_width || y >= m_height || x < 0 || y < 0 ) return;
    m_image.setPixel( x, y, QColor(color).rgb() );
}

void Display::updtImageSize()
{
    m_image = m_image.scaled( m_width, m_height );
    m_image.fill( m_background );
    this->setFixedSize( m_width*m_scale, m_height*m_scale );
}

void Display::paintEvent( QPaintEvent* )
{
    if( !m_scale ) return;

    QPainter p(this);

    p.drawImage( 0, 0, m_image.scaled( width(), height() ) );
}

