/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTWIDGET_H
#define VOLTWIDGET_H

#include "dialwidget.h"

class CustomButton;

class VoltWidget : public DialWidget
{
    public:
        VoltWidget();
        ~VoltWidget();

        CustomButton* pushButton;
};
#endif

