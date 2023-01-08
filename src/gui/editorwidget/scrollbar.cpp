/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QPaintEvent>

#include "scrollbar.h"
#include "codeeditor.h"

scrollWidget::scrollWidget( CodeEditor* editor, Qt::Orientation o, QWidget* parent )
            : QScrollBar( o, parent)
{
    m_editor = editor;
}
scrollWidget::~scrollWidget(){}

void scrollWidget::paintEvent( QPaintEvent* event )
{
    QScrollBar::paintEvent( event );

    QPainter painter;
    painter.begin(this);

    qreal w = width();
    qreal h = height()-2*w;

    qreal lines = m_editor->blockCount();
    for( int line : m_editor->getBreakPoints() )
    {
        qreal y = (line-1)*h/lines+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(255,100,0) );
    }
    for( int line : m_editor->getFound() )
    {
        qreal y = (line-1)*h/lines+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(200,200,50) );
    }
    painter.end();
}
