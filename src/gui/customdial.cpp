/*            USAGE

CustomDial* dial = new CustomDial(this);

dial->setStyleSheet("background-color: #27272B; color: #FFFFFF;");
dial->setStyleSheet("qproperty-knobRadius: 5; qproperty-knobMargin: 5;"); */

#include "customdial.h"

#include <QPainter>
#include <QColor>
#include <QPalette>
#include <QtMath>
#include <QStyleOptionSlider>

#define Q_PI 3,141592653589793

CustomDial::CustomDial( QWidget* parent )
          : QDial( parent )
{
    setFixedSize( 24, 24 );
    setRange( 0, 1000 );
    setValue( 500 );
    setSingleStep( 25 );
    setNotchesVisible( true );
    setAttribute( Qt::WA_TranslucentBackground );
}

void CustomDial::paintEvent( QPaintEvent* e )
{
    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing );    // Smooth out the circle
    QPen pen = painter.pen();

    int width = QDial::width();
    int height = QDial::height();
    qreal r = qMin(width, height)/2.0;
    qreal d = r/6.0;
    qreal dx = d + (width-2*r)/2.0 + 1;
    qreal dy = d + (height-2*r)/2.0 + 1;
    qreal fi = r*2 - d*2 -2;
    QRectF br = QRectF( dx+1, dy+1, fi-2, fi-2 );
    double knobRadius = fi/12.0;

    if( this->hasFocus() ) {
        painter.setOpacity( 0.3 );
        painter.setPen( QPen(Qt::NoPen) );
        QRadialGradient radialGrad( br.center(), width/2 );
        radialGrad.setColorAt( 0.7, Qt::blue );
        radialGrad.setColorAt( 1  , QColor( 240,240,210 ) );
        painter.setBrush( radialGrad );
        painter.drawEllipse( rect() );
        painter.setOpacity( 1 );
    }
    double degreer;
    double degrees;
    qreal spanAngle;
    qreal startAngle;
    int ticks;

    if( this->wrapping() ) {
       degreer = 2.0 * M_PI;   // Range 360°
       degrees = 1.5 * M_PI;   // Start 270°
       spanAngle = 360;
       startAngle = 270;
    }else {
       degreer = 1.66666667 * M_PI;   // Range 300°
       degrees = 1.33333333 * M_PI;   // Start 240°
       spanAngle = 300;
       startAngle = 240;
    }

    //Draw notches
    if( this->notchesVisible() )
    {
       ticks = this->maximum() / this->singleStep();
       if((fi * M_PI) / ticks < 4.0) {
          ticks = (fi * M_PI) / 4.0;
          if(ticks %2 != 0) ticks = ticks -1;
       }
       painter.save();
       pen.setColor( QColor(110,110,110) );
       painter.setPen( pen) ;
       painter.translate( width/2.0, height/2.0);
       painter.rotate(-startAngle);

       qreal angleStep = spanAngle/ticks;
       for( int i=0; i<=ticks; ++i) {
           painter.drawLine( QPointF(width/2-3, 0), QPointF(width/2, 0) );
           painter.rotate( angleStep );
       }
       painter.translate(-width/2, -height/2 );
       painter.restore();
    }
    pen.setWidth( 1 );
    pen.setColor( QColor(70,70,70) );
    painter.setPen( pen );
    painter.drawEllipse( QRectF( dx, dy, fi, fi ) );                          // Draw first circle

    pen.setColor( Qt::white );
    painter.setPen( pen );
    painter.drawEllipse( br );                          // Draw first circle

    QRadialGradient radialGrad( QPoint(dx, dy), fi);
    radialGrad.setColorAt( 0   , QColor( 255,255,255 ) );
    radialGrad.setColorAt( 0.80, QColor( 230,230,225 ) );
    radialGrad.setColorAt( 0.83, QColor( 220,220,215 ) );
    radialGrad.setColorAt( 1   , QColor( 200,200,195 ) );
    painter.setBrush( radialGrad );
    painter.setPen( QPen(Qt::NoPen) );
    painter.drawEllipse( br );                          // Draw first circle

    double ratio = double(QDial::value())/QDial::maximum();    // Get ratio between current value and maximum to calculate angle
    double angle = ratio*degreer - degrees;                  // The maximum amount of degrees is 270, offset by 225

    // Add r to have (0,0) in center of dial
    double y = sin(angle)*(fi/2-knobRadius*2.5) + r;
    double x = cos(angle)*(fi/2-knobRadius*2.5) + r;

    // Draw the knob ellipse
    pen.setColor( QColor( 240, 240, 230) );
    painter.setPen( pen );
    painter.drawEllipse( QPointF(x,y), knobRadius+0.5, knobRadius+0.5 );
    painter.setBrush( QColor(210,210,200) );
    pen.setColor( QColor(70,70,70) );
    painter.setPen( pen );
    painter.drawEllipse( QPointF(x,y), knobRadius, knobRadius );
    painter.setPen( QPen(Qt::NoPen) );
    painter.drawEllipse( QPointF(x,y), knobRadius-0.2, knobRadius-0.2 );
}
