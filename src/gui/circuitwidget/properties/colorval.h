/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COLORVAL_H
#define COLORVAL_H

#include "ui_colorval.h"
#include "propval.h"

class Component;
class PropDialog;

class ColorVal : public PropVal, private Ui::ColorVal
{
    Q_OBJECT
    
    public:
        ColorVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~ColorVal();

        virtual void setup() override;
        virtual void updtValues() override;

    public slots:
        void on_valueBox_currentIndexChanged( int index );

    private:
        QColor m_color;
};

#endif
