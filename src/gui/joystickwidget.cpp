/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include <QVariant>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QtMath>

#include "joystickwidget.h"

#define STICK_PERCENTAGE_SIZE 0.6

JoystickWidget::JoystickWidget()
{
    installEventFilter(this);
}
JoystickWidget::~JoystickWidget() {}

void JoystickWidget::setupWidget( int size )
{
    setFixedSize( size, size );
    m_size = size;
    m_maxDistance = (m_size-24)/2;
    
    m_xValue = 500;
    m_yValue = 500;

    m_center = QPointF( size/2, size/2 );
    m_knobCenter = m_center;

    int radius = (m_size-16)*STICK_PERCENTAGE_SIZE/2;
    m_knobArea = QRectF(-radius, -radius, 2*radius, 2*radius );
    
    m_grabCenter = false;
    
    setAttribute( Qt::WA_NoSystemBackground ) ;

    updateOutputValues();
}

void JoystickWidget::updateOutputValues()
{
    m_xValue = (m_knobCenter.x()-12) * 500 / m_maxDistance;
    m_yValue = (m_knobCenter.y()-12) * 500 / m_maxDistance;

    m_changed = true;
    update();
}

bool JoystickWidget::eventFilter( QObject*, QEvent* event )
{
    if( event->type() != QEvent::MouseButtonPress ) return false;

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if( mouseEvent->buttons() != Qt::LeftButton ) return false;

    double distance = QLineF( m_center, mouseEvent->pos() ).length();

    if( distance < 10 ) {
        m_grabCenter = true;
        event->accept();
    }
    else event->ignore();

    return true;
}

void JoystickWidget::mouseMoveEvent( QMouseEvent* event )
{
    if( !m_grabCenter ) return;

    QLineF limitLine = QLineF( m_center, event->pos() );
    if( limitLine.length() > m_maxDistance ) limitLine.setLength( m_maxDistance );

    m_knobCenter = limitLine.p2();
    updateOutputValues();
}

void JoystickWidget::mouseReleaseEvent( QMouseEvent* event )
{
    m_grabCenter = false;
    m_knobCenter = m_center;
    updateOutputValues();
}

void JoystickWidget::paintEvent( QPaintEvent* )
{
    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing );

    int size = m_size-16;
    int x0 = m_center.x();
    int y0 = m_center.y();
    QRectF bounds = QRectF(-size/2,-size/2, size, size ).translated( m_center );
    QRadialGradient lg0( QPointF( x0-3, y0-3 ), 20, QPointF( x0-30, y0-30 ) );
    lg0.setColorAt( 0, QColor( 100, 100, 100 ) );
    lg0.setColorAt( 1, QColor( 0, 0, 0 ) );
    painter.setBrush( lg0 );
    painter.drawEllipse( bounds );

    QLineF knobLine = QLineF( m_knobCenter, m_center );
    knobLine.setLength( knobLine.length() * 0.7 );
    QPointF center = knobLine.p2();

    painter.setBrush( QColor( 30, 30, 30 ) );
    painter.setPen( QPen( QColor( Qt::black ), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
    painter.drawEllipse( QRectF(-7,-7, 14, 14 ).translated( center ) );

    painter.setPen( QPen( QColor( Qt::black ), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );

    QRectF knob = m_knobArea.translated( m_knobCenter );
    x0 = knob.x();
    y0 = knob.y();

    QRadialGradient lg( QPointF(x0+10, y0 ), 17, QPointF( x0-5, y0 ) );
    lg.setColorAt( 0, QColor( 180, 180, 180 ) );
    lg.setColorAt( 1, QColor( 50, 50, 50 ) );
    painter.setBrush( lg );
    painter.drawEllipse( knob );
}
