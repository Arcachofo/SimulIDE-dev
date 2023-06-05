/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "customslider.h"

#include <QPainter>
//#include <QColor>
//#include <QPalette>
//#include <QSlider>
//#include <QtMath>
//#include <QStyleOptionSlider>

CustomSlider::CustomSlider( QWidget* parent )
            : QSlider( Qt::Horizontal, parent )
{
    setFixedSize( 48, 14 );
    setRange( 0, 1000 );
    setValue( 500 );
    setSingleStep( 25 );
    //setTickInterval( 500 );
    //setTickPosition( QSlider::TicksAbove );
    setAttribute( Qt::WA_TranslucentBackground );
}

void CustomSlider::paintEvent( QPaintEvent* e )
{
    //QSlider::paintEvent( e );

    QPainter painter( this );

    int w = width();
    int h = height();
    int pos = this->value()*(w-12)/1000+6;

    painter.fillRect( QRectF( 1, h/2-1, pos, 2), Qt::darkGreen );
    painter.fillRect( QRectF( pos, h/2-1, w-1, 2), Qt::darkGray );

    QPen pen = painter.pen();
    pen.setColor( QColor( 110, 110, 110 ) );
    painter.setPen( pen );
    painter.setBrush( QColor( 230, 230, 230 ) );
    painter.drawRoundedRect( QRectF( pos-h/2+1.5, 1, h-3, h-3), 2 ,2 );
}
