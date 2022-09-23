/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BOOLVAL_H
#define BOOLVAL_H

#include "ui_boolval.h"
#include "propval.h"

class Component;
class PropDialog;

class BoolVal : public PropVal, private Ui::BoolVal
{
    Q_OBJECT
    
    public:
        BoolVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~BoolVal();
        
        virtual void setup() override;
        virtual void updtValues() override;

    public slots:
        void on_trueVal_toggled( bool checked );
};

#endif
