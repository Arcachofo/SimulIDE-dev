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

#include <math.h>

#include "dspinbox.h"

asIScriptEngine* DSpinBox::m_aEngine = asCreateScriptEngine();

DSpinBox::DSpinBox( QWidget* parent )
        : QDoubleSpinBox( parent )
{}
DSpinBox::~DSpinBox(){;}


QString DSpinBox::textFromValue( double value ) const
{
    return QString::number( value );
}

QValidator::State DSpinBox::validate( QString& text, int& ) const
{
    return isnan( valueFromText( text ) ) ? QValidator::Invalid : QValidator::Acceptable;
}

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

double DSpinBox::evaluate( QString exp )
{
    exp = "return "+ exp;
    const char* code = exp.toLocal8Bit().data();
    double res = 0;
    int r = ExecuteString( m_aEngine, code, &res, asTYPEID_DOUBLE );
    //qDebug()<<"----\n" <<r<<code<<  "=" << res;
    if( r >= 0 ) return res;
    return NAN ;
}

