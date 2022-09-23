/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SRTINGVAL_H
#define SRTINGVAL_H

#include "ui_stringval.h"
#include "propval.h"

class Component;
class PropDialog;

class StringVal : public PropVal, private Ui::StringVal
{
    Q_OBJECT
    
    public:
        StringVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~StringVal();

        virtual void setup() override;
        virtual void updtValues() override;

    public slots:
        void on_value_editingFinished();
};

#endif
