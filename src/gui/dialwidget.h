/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIALWIDGET_H
#define DIALWIDGET_H

#include <QWidget>

class QAbstractSlider;
class QVBoxLayout;
class CustomDial;
class CustomSlider;

class DialWidget : public QWidget
{
    Q_OBJECT
    
    public:
        DialWidget();
        ~DialWidget();

        void setSize( int size );

        void setType( int type );

        void setValue( int v );
        int value();

        QAbstractSlider* dial() { return m_dial; }

    protected:
        virtual void paintEvent( QPaintEvent* e) override;

        QVBoxLayout* m_verticalLayout;

        QAbstractSlider* m_dial;
        CustomDial*      m_knob;
        CustomSlider*    m_slider;
};
#endif

