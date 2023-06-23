/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "custombutton.h"
#include "mainwindow.h"

#include <QPainter>
#include <QColor>

CustomButton::CustomButton( QWidget* parent )
            : QToolButton( parent )
{
    QFont f = this->font();
    f.setFamily("Ubuntu");
    f.setPixelSize(11);
    setFont( f );

    setAttribute( Qt::WA_TranslucentBackground );
}

void CustomButton::paintEvent( QPaintEvent* )
{
    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing ); // Smooth out the circle
    QPen pen = painter.pen();

    int h = height()-2;
    int w = width();

    pen.setColor( QColor( 110, 110, 110 ) );
    painter.setPen( pen );
    painter.drawRoundedRect( QRectF( .8, .8, w-2+.4, h+.4 ),2,2 );
    painter.setPen( QPen(Qt::NoPen) );

    QRectF rect;
    QLinearGradient linearGrad( QPointF(w/2, 0), QPointF(w/2, h) );

    if( this->isDown() || isChecked() ){
        linearGrad.setColorAt( 0, QColor( 230, 230, 230 ) );
        linearGrad.setColorAt( 1, QColor( 180, 180, 180 ) );
        rect = QRectF( 1.3, 1.3, w-2, h );
    }else{
        linearGrad.setColorAt(0, Qt::white);
        linearGrad.setColorAt(1, QColor( 200, 200, 200 ) );
        rect = QRectF( 1, 1, w-2, h );
    }
    painter.setBrush( linearGrad );
    painter.drawRoundedRect( rect, 2, 2 );

    pen.setColor( QColor( 0, 0, 20 ) );
    painter.setPen( pen );
    painter.drawText( rect, Qt::AlignCenter, text() );
}
