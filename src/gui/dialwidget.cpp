/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDial>
#include <QVBoxLayout>

#include "dialwidget.h"
#include "customdial.h"
#include "customslider.h"

DialWidget::DialWidget()
{
    m_slider = NULL;
    m_knob = new CustomDial( this );
    m_dial = m_knob;

    m_verticalLayout = new QVBoxLayout( this );
    m_verticalLayout->setContentsMargins( 0, 0, 0, 0 );
    m_verticalLayout->setSpacing( 0 );
    m_verticalLayout->addWidget( m_knob );

    setAttribute( Qt::WA_TranslucentBackground );
}
DialWidget::~DialWidget() {}

void DialWidget::setSize( int size )
{
    if( m_dial == m_knob ) m_knob->setFixedSize( size, size );
    else                   m_slider->setFixedSize( size, 12 );
}

void DialWidget::setType( int type )
{
    if( type == 0 ) // Knob
    {
        m_dial = m_knob;
        if( m_slider ) m_slider->setVisible( false );
        m_knob->setVisible( true );
    }
    else            // Slider
    {
        m_verticalLayout->removeWidget( m_knob );
        if( !m_slider )
        {
            m_slider = new CustomSlider( this );
            m_verticalLayout->addWidget( m_slider );
        }
        m_dial = m_slider;

        m_knob->setVisible( false );
        m_slider->setVisible( true );
    }
    this->setFixedSize( m_dial->size() );
}

void DialWidget::setValue( int v )
{
    m_dial->setValue( v );
}

int DialWidget::value()
{
    return m_dial->value();
}

void DialWidget::paintEvent( QPaintEvent* e )
{
}

#include "moc_dialwidget.cpp"
