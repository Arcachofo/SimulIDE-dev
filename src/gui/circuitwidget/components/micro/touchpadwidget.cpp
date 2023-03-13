/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMouseEvent>

#include "touchpadwidget.h"

TouchPadWidget::TouchPadWidget()
{
    setCursor( Qt::PointingHandCursor );
}
TouchPadWidget::~TouchPadWidget() {}

void TouchPadWidget::mousePressEvent( QMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_xMousePos = event->x();
        m_yMousePos = event->y();
        checkValues();
    }
    else QWidget::mousePressEvent( event );
}

void TouchPadWidget::mouseMoveEvent( QMouseEvent* event )
{
    m_xMousePos = event->x();
    m_yMousePos = event->y();
    checkValues();
}

void TouchPadWidget::mouseReleaseEvent( QMouseEvent* event )
{
    resetValues();
}

void TouchPadWidget::checkValues()
{
    if( m_xMousePos < 0 || m_xMousePos >width()
     || m_yMousePos < 0 || m_yMousePos >height() )
        resetValues();
}

void TouchPadWidget::resetValues()
{
    m_xMousePos = -1;
    m_yMousePos = -1;
}
