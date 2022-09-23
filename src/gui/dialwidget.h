/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIALWIDGET_H
#define DIALWIDGET_H

#include <QWidget>

class QDial;
class QVBoxLayout;

class DialWidget : public QWidget
{
    Q_OBJECT
    
    public:
        DialWidget();
        ~DialWidget();

        void setupWidget();

        QDial* dial;
        QVBoxLayout* verticalLayout;
};
#endif

