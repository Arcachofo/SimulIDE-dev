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
    painter.setRenderHint( QPainter::Antialiasing );

    qreal lines = m_editor->blockCount();
    qreal w = width();
    qreal h = (height()-2*w)/lines;

    for( int line : *m_editor->getBreakPoints() )
    {
        qreal y = (line-1)*h+w;
        painter.setOpacity( 0.5 );
        painter.fillRect( QRectF( 0, y-2, w, 4 ), QColor(50, 50, 0) );
        painter.setOpacity( 1 );
        painter.fillRect( QRectF( 0, y-1.5, w, 3 ), QColor(255, 255, 20) );
    }
    for( int line : m_editor->getFound() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y-1, w, 2 ), QColor(80, 80, 255) );
    }
    for( int line : *m_editor->getErrors() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y-1, w, 2 ), QColor(Qt::red) );
    }
    for( int line : *m_editor->getWarnings() )
    {
        qreal y = (line-1)*h+w;
        painter.fillRect( QRectF( 0, y-1, w, 2 ), QColor(240, 140, 0) );
    }
    int debugLine = m_editor->debugLine();
    if( debugLine > 0 )
    {
        qreal y = (debugLine-1)*h+w;
        qreal w2 = w-2;
        painter.drawImage( QRectF( 0, y-w2/2, w2, w2 ), QImage(":/brkpoint.png") );
    }
    painter.end();
}
