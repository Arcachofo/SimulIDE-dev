/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
    m_xMousePos = width()/2;
    m_yMousePos = height()/2;
}

#include "moc_touchpadwidget.cpp"
