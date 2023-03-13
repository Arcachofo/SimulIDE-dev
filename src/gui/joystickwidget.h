/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>

class JoystickWidget : public QWidget
{
    public:
        JoystickWidget();
        ~JoystickWidget();

        void setupWidget();
        
        int getXValue() { return m_xValue; }
        int getYValue() { return m_yValue; }
        bool changed()  { return m_changed; }
        
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );
        void paintEvent( QPaintEvent* event );

    private:
        int m_xValue;
        int m_yValue;

        bool m_changed;
        
        QPointF m_movingOffset;
        bool m_grabCenter;
        
        QRectF centerEllipse();
        QPointF center();
        void updateOutputValues();
};
#endif

