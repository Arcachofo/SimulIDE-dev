/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef NUMVAL_H
#define NUMVAL_H

#include "ui_numval.h"
#include "propval.h"

class Component;
class PropDialog;

class NumVal : public PropVal, private Ui::NumVal
{
    Q_OBJECT
    
    public:
        NumVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~NumVal();

        virtual void setup() override;
        virtual void updtValues() override;

        virtual QString getValWithUnit() override;

    public slots:
        void on_showVal_toggled( bool checked );
        void on_valueBox_valueChanged( double val );
        void on_unitBox_currentTextChanged( QString unit );

    private:
        QString m_type;

        bool m_useMult;
};

#endif
