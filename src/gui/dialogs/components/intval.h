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

#include <QWidget>

#include "ui_intval.h"

#ifndef INTVAL_H
#define INTVAL_H

class Component;

class IntVal : public QWidget, private Ui::IntVal
{
    Q_OBJECT
    
    public:
        IntVal( QWidget* parent=0 );
        
        void setup( Component* comp, QString unit );
        void setPropName(QString name, QString caption );

    public slots:
        void on_value_valueChanged( int val );
        void on_unitBox_currentTextChanged( QString unit );

    private:
        void updateValues();

        Component* m_component;
        QString m_propName;

        bool m_blocked;
};

#endif
