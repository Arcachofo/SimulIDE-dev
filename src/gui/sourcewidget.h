/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include "dialwidget.h"

class CustomButton;

class SourceWidget : public DialWidget
{
    public:
        SourceWidget();
        ~SourceWidget();

        CustomButton* pushButton;
};
#endif

