/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TOUCHPADWIDGET_H
#define TOUCHPADWIDGET_H

#include <QWidget>

class TouchPadWidget : public QWidget
{
    public:
        TouchPadWidget();
        ~TouchPadWidget();
        
        void resetValues();
        int getXValue() { return m_xMousePos; }
        int getYValue() { return m_yMousePos; }

    protected:
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );

    private:
        void checkValues();

        int m_xMousePos;
        int m_yMousePos;
};
#endif
