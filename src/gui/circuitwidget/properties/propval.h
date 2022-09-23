/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PROPVAL_H
#define PROPVAL_H

#include <QWidget>

class Component;
class PropDialog;
class QComboBox;
class ComProperty;

class PropVal : public QWidget
{
    public:
        PropVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~PropVal();

        virtual void setup()=0;
        virtual void updtValues() {;}

        virtual QString getValWithUnit() { return ""; }

        QString propName() { return m_propName; }

    protected:
        void addDividers( QComboBox* unitBox, QString unit );
        void addMultipliers( QComboBox* unitBox, QString unit );

        bool m_blocked;

        QString m_propName;

        Component*   m_component;
        ComProperty* m_property;
        PropDialog*  m_propDialog;
};

#endif

