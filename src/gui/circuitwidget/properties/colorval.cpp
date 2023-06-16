/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QColorDialog>

#include "colorval.h"
#include "component.h"
#include "comproperty.h"
#include "propdialog.h"

ColorVal::ColorVal( PropDialog* parent, Component* comp, ComProperty* prop )
        : PropVal( parent, comp, prop )
{
    setupUi(this);
}
ColorVal::~ColorVal(){}

void ColorVal::setup()
{
    valLabel->setText( m_property->capt() );

    m_color = m_property->getValStr(); /// FIXME // comp->property( m_propName.toUtf8() ).value<QColor>();
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

    m_component->setPropStr( m_propName, m_color.name() ); /// FIXME

    QPalette pal = colorW->palette();
    pal.setColor( QPalette::Base, m_color );
    colorW->setPalette( pal );

    m_propDialog->changed();
}

void ColorVal::updtValues()
{

}

