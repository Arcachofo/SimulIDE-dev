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

#include "colorval.h"
#include "component.h"

ColorVal::ColorVal( QWidget* parent )
        : PropVal( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
}
ColorVal::~ColorVal() {;}

void ColorVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void ColorVal::setup( Component* comp, QString )
{
    m_color = comp->property( m_propName.toUtf8() ).value<QColor>();
    QString cname = m_color.name();
    int index = 0;

    valueBox->addItem( "Custom" );

    QStringList colorNames = QColor::colorNames();
    for( int i=0; i<colorNames.size(); ++i )
    {
        QString colorName = colorNames[i];
        QColor color( colorName );
        if( color.name() == cname ) index = i+1;
        valueBox->addItem( colorName );
        valueBox->setItemData( i+1, color, Qt::DecorationRole );
    }
    valueBox->setEditable( true );
    valueBox->setCurrentIndex( index );
    QPalette pal = colorW->palette();
    pal.setColor( QPalette::Base, m_color );
    colorW->setPalette( pal );

    m_component = comp;
    this->adjustSize();
}

void ColorVal::on_valueBox_currentIndexChanged( int index )
{
    if( !m_component ) return;

    QString value = valueBox->itemText( index );
    if( value == "Custom" )
    {
        QColor color = QColorDialog::getColor( m_color, this );
        if( color.isValid() )
        {
            m_color = color;
            valueBox->setItemData( 0, color, Qt::DecorationRole );
        }
    }
    else m_color = QColor( value );

    m_component->setProperty( m_propName.toUtf8(), m_color );

    QPalette pal = colorW->palette();
    pal.setColor( QPalette::Base, m_color );
    colorW->setPalette( pal );
}

void ColorVal::updtValues()
{

}

