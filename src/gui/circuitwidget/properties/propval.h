/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PROPVAL_H
#define PROPVAL_H

#include <QWidget>

class CompBase;
class PropDialog;
class QComboBox;
class ComProperty;

class PropVal : public QWidget
{
    public:
        PropVal( PropDialog* parent, CompBase* comp, ComProperty* prop );
        ~PropVal();

        virtual void setup( bool isComp )=0;
        virtual void updtValues() {;}

        virtual QString getValWithUnit() { return ""; }

        QString propName() { return m_propName; }

    protected:
        void addDividers( QComboBox* unitBox, QString unit );
        void addMultipliers( QComboBox* unitBox, QString unit );

        void prepareChange();
        void saveChanges();

        bool m_blocked;
        bool m_undo;
        QString m_oldValue;

        QString m_propName;

        CompBase*    m_component;
        ComProperty* m_property;
        PropDialog*  m_propDialog;
};

#endif
