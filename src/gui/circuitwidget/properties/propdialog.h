/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDialog>

#include "ui_propdialog.h"

#ifndef PROPDIALOG_H
#define PROPDIALOG_H

class Component;
class PropVal;

class PropDialog : public QDialog, private Ui::PropDialog
{
    Q_OBJECT
    
    public:
        PropDialog( QWidget* parent=0, QString help="" );

        void setComponent( Component* comp );
        void showProp( QString name, bool show );
        void updtValues();

        void changed();

    public slots:
        void on_labelBox_editingFinished();
        void on_showLabel_toggled( bool checked );
        void on_tabList_currentChanged( int );
        void on_helpButton_clicked();
        void adjustWidgets();

    private:
        Component* m_component;

        bool m_helpExpanded;

        float m_scale;
        int m_minW;
        int m_minH;

        QList<PropVal*> m_propList;
};


#endif
