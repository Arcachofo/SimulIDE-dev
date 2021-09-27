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

