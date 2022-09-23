/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTWIDGET_H
#define VOLTWIDGET_H

#include "dialwidget.h"

class QPushButton;

class VoltWidget : public DialWidget
{
    public:
        VoltWidget();
        ~VoltWidget();

        void setupWidget();

        QPushButton* pushButton;

};
#endif

