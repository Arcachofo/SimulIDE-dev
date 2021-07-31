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

#ifndef SRTINGVAL_H
#define SRTINGVAL_H

#include "ui_stringval.h"
#include "propval.h"

class Component;

class StringVal : public PropVal, private Ui::StringVal
{
    Q_OBJECT
    
    public:
        StringVal( QWidget* parent=0 );
        ~StringVal();

        virtual void setup( Component* comp, QString ) override;
        virtual void setPropName( QString name, QString caption ) override;
        virtual void updtValues() override;

    public slots:
        void on_value_editingFinished();

    private:
        void updatValue();

        Component* m_component;
        QString m_propName;
};

#endif
