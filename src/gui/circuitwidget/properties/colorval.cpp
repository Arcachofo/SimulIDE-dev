/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QColorDialog>

#include "colorval.h"
#include "component.h"
#include "comproperty.h"
#include "propdialog.h"
//#include "mainwindow.h"

ColorVal::ColorVal( PropDialog* parent, CompBase* comp, ComProperty* prop )
        : PropVal( parent, comp, prop )
{
    setupUi(this);
    m_blocked = true;
}
ColorVal::~ColorVal(){}

void ColorVal::setup( bool )
{
    //float scale = MainWindow::self()->fontScale();
    //QFont font = valLabel->font();
    //font.setPixelSize( 11.0*scale );
    //valLabel->setFont( font );
    //colorW->setFont( font );
    //valueBox->setFont( font );

    QFontMetrics fm( valueBox->font() );
    float scale = fm.width(" ")/2;
    valueBox->setFixedWidth( 170.0*scale );

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

    m_blocked = false;
}

void ColorVal::on_valueBox_activated( int index )
{
    if( m_blocked ) return;
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

