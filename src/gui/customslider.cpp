/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "customslider.h"

#include <QPainter>
//#include <QDebug>

CustomSlider::CustomSlider( QWidget* parent )
            : QSlider( Qt::Horizontal, parent )
{
    setFixedSize( 60, 14 );
    setRange( 0, 1000 );
    setValue( 500 );
    setSingleStep( 1 );
    setAttribute( Qt::WA_TranslucentBackground );
}

void CustomSlider::paintEvent( QPaintEvent* )
{
    QPainter painter( this );

    int w = width();
    int h = height();
    int pos = this->value()*(w-12)/1000+6;

    QPen pen = painter.pen();
    pen.setColor( QColor( 60, 60, 60 ) );
    pen.setWidthF( 0.8 );
    painter.setPen( pen );

    QLinearGradient linearGrad( QPointF(w/2, 0), QPointF(w/2, h) );
    linearGrad.setColorAt(0, Qt::white);
    linearGrad.setColorAt(1, QColor( 200, 200, 200 ) );
    painter.setBrush( linearGrad );

    painter.drawRoundedRect( QRectF( pos-h/2+1.5, 1.5, h-3, h-3), 2, 2 );

    pen.setColor( QColor( 110, 110, 110 ) );
    painter.setPen( pen );
    //Arrow
    painter.setBrush( QColor( 170, 170, 170 ) );
    QPointF points[3] = {
            QPointF( pos-2, 1.5 ),
            QPointF( pos, 4),
            QPointF( pos+2, 1.5 ) };
    painter.drawPolygon(points, 3);

    QPointF points2[3] = {
            QPointF( pos-2, h-1.5 ),
            QPointF( pos, h-4),
            QPointF( pos+2, h-1.5 ) };
    painter.drawPolygon(points2, 3);

    // Rail
    painter.fillRect( QRectF( 6, h/2-1, pos-6, 2), Qt::darkGreen );
    painter.fillRect( QRectF( pos, h/2-1, w-6-pos, 2), QColor( 140, 140, 140 ) );

    // Notches
    painter.drawLine( QPointF( 6, h/2-2 ), QPointF( 6, h/2+2) );
    painter.drawLine( QPointF( (w-12)/4+6, h/2-1.5 ), QPointF( (w-12)/4+6, h/2+1.5) );
    painter.drawLine( QPointF( w/2, h/2-2 ), QPointF( w/2, h/2+2) );
    painter.drawLine( QPointF( 3*(w-12)/4+6, h/2-1.5 ), QPointF( 3*(w-12)/4+6, h/2+1.5) );
    painter.drawLine( QPointF( w-6, h/2-2 ), QPointF( w-6, h/2+2) );

    painter.setOpacity( 0.6 );
    painter.setBrush( linearGrad );
    painter.drawRoundedRect( QRectF( pos-h/2+1.5, 1.5, h-3, h-3), 2, 2 );
    painter.setOpacity( 1 );

}
