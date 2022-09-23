/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>

class JoystickWidget : public QWidget
{
    Q_OBJECT
    
    public:
        JoystickWidget();
        ~JoystickWidget();

        void setupWidget();
        
        int getXValue() { return m_xValue; }
        int getYValue() { return m_yValue; }
        
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );
        void paintEvent( QPaintEvent* event );
        
    signals:
        void valueChanged(int newXValue, int newYValue);
        
    private:
        int m_xValue;
        int m_yValue;
        
        QPointF m_movingOffset;
        bool m_grabCenter;
        
        QRectF centerEllipse();
        QPointF center();
        void updateOutputValues();
};
#endif

