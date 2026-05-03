/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#pragma once

#include <QWidget>

class JoystickWidget : public QWidget
{
    public:
        JoystickWidget();
        ~JoystickWidget();

        void setupWidget( int size );
        
        QPointF getPos() { return QPointF( m_xValue, m_yValue ); m_changed = false; }
        bool   changed() { return m_changed; }

        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );
        void paintEvent( QPaintEvent* event );

    private:
        bool eventFilter( QObject* object, QEvent* event ) override;
        void updateOutputValues();

        bool m_changed;

        int m_size;
        int m_maxDistance;

        double m_xValue;
        double m_yValue;

        QPointF m_center;
        
        QRectF m_knobArea;
        QPointF m_knobCenter;
        bool m_grabCenter;
};
