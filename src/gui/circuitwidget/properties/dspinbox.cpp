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

#include <QScriptEngine>
#include <math.h>

#include "dspinbox.h"

DSpinBox::DSpinBox( QWidget* parent )
        : QDoubleSpinBox( parent )
{}
DSpinBox::~DSpinBox(){;}

double DSpinBox::valueFromText( const QString& text ) const
{
    QString exp = text;
    if( exp.indexOf("e") == exp.size()-1 ) exp.append("0");
    if( exp.indexOf("+") == exp.size()-1 ) exp.append("0");
    if( exp.indexOf("-") == exp.size()-1 ) exp.append("0");
    if( exp.indexOf("*") == exp.size()-1 ) exp.append("1");
    if( exp.indexOf("/") == exp.size()-1 ) exp.append("1");

    return evaluate( exp );
}

QString DSpinBox::textFromValue( double value ) const
{
    return QString::number( value );
}

QValidator::State DSpinBox::validate( QString& text, int& ) const
{
    return std::isnan( valueFromText( text ) ) ? QValidator::Invalid : QValidator::Acceptable;
}

double DSpinBox::evaluate( QString exp )
{
 static QScriptEngine expression;

    return expression.evaluate( exp ).toNumber();
}

