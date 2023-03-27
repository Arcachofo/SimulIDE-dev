/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>

class JoystickWidget : public QWidget
{
    public:
        JoystickWidget();
        ~JoystickWidget();

        void setupWidget();
        
        double getXValue() { return m_xValue; }
        double getYValue() { return m_yValue; }
        bool   changed()   { return m_changed; }
        
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );
        void paintEvent( QPaintEvent* event );

    private:
        double m_xValue;
        double m_yValue;

        bool m_changed;
        
        QPointF m_movingOffset;
        bool m_grabCenter;
        
        QRectF centerEllipse();
        QPointF center();
        void updateOutputValues();
};
#endif

