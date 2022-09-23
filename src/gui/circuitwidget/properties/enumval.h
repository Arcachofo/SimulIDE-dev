/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ENUMVAL_H
#define ENUMVAL_H

#include "ui_enumval.h"
#include "propval.h"

class Component;
class PropDialog;

class EnumVal : public PropVal, private Ui::EnumVal
{
    Q_OBJECT
    
    public:
        EnumVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~EnumVal();

        virtual void setup() override;
        virtual void updtValues() override;

    public slots:
        void on_showVal_toggled( bool checked );
        void on_valueBox_currentIndexChanged( QString val );

    protected:
        QStringList m_enums;
};

#endif
