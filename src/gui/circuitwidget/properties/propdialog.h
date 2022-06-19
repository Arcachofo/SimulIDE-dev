/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
