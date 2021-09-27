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
#include "comproperty.h"

TextVal::TextVal( PropDialog* parent, Component* comp, ComProperty* prop )
       : PropVal( parent, comp, prop )
{
    setupUi(this);
}
TextVal::~TextVal() {}

void TextVal::setup()
{
    valLabel->setText( m_property->capt() );
    m_scriptComp = (Scripted*)m_component;
    updatValue();
    this->adjustSize();
}

void TextVal::on_saveButton_clicked()
{
    QString script = textBox->toPlainText();
    QString excep  = m_scriptComp->evaluate( script );

    if( excep == "" )
    {
        excep = tr(" No Errors\n\n    Script Saved.");
        /// FIXME m_scriptComp->setProperty( m_propName.toUtf8(), script );
    }
    else excep += tr("\n\n    Script Not Saved.");

    evalText->setPlainText( excep );
}

void TextVal::updatValue()
{
    /// FIXME QString text = m_scriptComp->property( m_propName.toUtf8() ).toString();
    /// FIXME textBox->setPlainText( text );
}

void TextVal::updtValues()
{

}
