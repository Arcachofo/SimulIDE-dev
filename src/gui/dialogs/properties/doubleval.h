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

#ifndef DOUBLEVAL_H
#define DOUBLEVAL_H

#include "ui_doubleval.h"
#include "propval.h"

class Component;

class DoubleVal : public PropVal, private Ui::DoubleVal
{
    Q_OBJECT
    
    public:
        DoubleVal( QWidget* parent=0 );
        ~DoubleVal();

        virtual void setup( Component* comp, QString unit ) override;
        virtual void setPropName( QString name, QString caption ) override;
        virtual void updtValues() override;

    public slots:
        void on_valueBox_valueChanged( double val );
        void on_showVal_toggled( bool checked );
        void on_unitBox_currentTextChanged( QString unit );

    private:
        void setUnit();

        Component* m_component;
        QString m_propName;

        bool m_blocked;
        bool m_main;
};


#endif
