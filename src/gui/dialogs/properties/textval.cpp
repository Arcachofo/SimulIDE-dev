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

#include "textval.h"
#include "scripted.h"

TextVal::TextVal( QWidget* parent )
       : PropVal( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
}
TextVal::~TextVal() {}

void TextVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void TextVal::setup( Component* comp, QString )
{
    m_component = (Scripted*)comp;
    updatValue();
    this->adjustSize();
}

void TextVal::on_saveButton_clicked()
{
    QString script = textBox->toPlainText();
    QString excep = m_component->evaluate( script );

    if( excep == "" )
    {
        excep = tr(" No Errors\n\n    Script Saved.");
        m_component->setProperty( m_propName.toUtf8(), script );
    }
    else excep += tr("\n\n    Script Not Saved.");

    evalText->setPlainText( excep );
}

void TextVal::updatValue()
{
    QString text = m_component->property( m_propName.toUtf8() ).toString();
    textBox->setPlainText( text );
}

void TextVal::updtValues()
{

}
