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

#ifndef TEXTVAL_H
#define TEXTVAL_H

#include "ui_textval.h"
#include "propval.h"

class Component;
class PropDialog;
class Scripted;

class TextVal : public PropVal, private Ui::TextVal
{
    Q_OBJECT
    
    public:
        TextVal( PropDialog* parent, Component* comp, ComProperty* prop );
        ~TextVal();
        
        virtual void setup() override;
        virtual void updtValues() override;

    public slots:
        void on_saveButton_clicked();

    private:
        void updatValue();

        Scripted* m_scriptComp;
};

#endif
