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

    QFontMetrics fm( colorW->font() );
    float scale = fm.width(" ")/2;
    colorW->setFixedWidth( 170.0*scale );

    valLabel->setText( m_property->capt() );

    m_color = m_property->getValStr(); /// FIXME // comp->property( m_propName.toUtf8() ).value<QColor>();

    QPalette pal = colorW->palette();
    pal.setColor( QPalette::Base, m_color );
    colorW->setPalette( pal );
    //colorW->setText( m_color.name() );
    colorW->setReadOnly( true );
    colorW->installEventFilter(this);

    this->adjustSize();

    m_blocked = false;
}

bool ColorVal::eventFilter( QObject* object, QEvent* event)
{
    if( object == colorW && event->type() == QEvent::MouseButtonPress ) {
        changeColor();
    }
    return false;
}

void ColorVal::changeColor()
{
    if( m_blocked ) return;
    if( !m_component ) return;

    QColor color = QColorDialog::getColor( m_color, this );
    if(!color.isValid() ) return;
    m_color = color;

    m_component->setPropStr( m_propName, m_color.name() ); /// FIXME

    QPalette pal = colorW->palette();
    pal.setColor( QPalette::Base, m_color );
    colorW->setPalette( pal );
    //colorW->setText( m_color.name() );

    m_propDialog->changed();
}

void ColorVal::updtValues()
{

}

