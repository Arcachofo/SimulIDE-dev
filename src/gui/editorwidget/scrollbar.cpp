/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QPaintEvent>

#include "scrollbar.h"
#include "codeeditor.h"
#include "basedebugger.h"

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

    qreal lines = m_editor->blockCount();
    qreal w = width();
    qreal h = (height()-2*w)/lines;

    for( int line : *m_editor->getBreakPoints() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(210, 210, 0) );
    }
    for( int line : m_editor->getFound() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(Qt::blue) );
    }
    for( int line : *m_editor->getErrors() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(Qt::red) );
    }
    for( int line : *m_editor->getWarnings() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y, w, 2 ), QColor(240, 140, 0) );
    }

    painter.end();
}
