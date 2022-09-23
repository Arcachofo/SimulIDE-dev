/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QToolButton>
#include <QGraphicsProxyWidget>

#include "switch_base.h"
#include "simulator.h"
#include "circuit.h"
#include "label.h"

SwitchBase::SwitchBase( QObject* parent, QString type, QString id )
          : MechContact( parent, type, id )
{
    m_graphical = true;
    m_changed = true;
    m_key = "";

    m_area =  QRectF( 0,0,0,0 );
    m_ePin.resize(2);
    m_idLabel->setPos(-12,-24);

    m_button = new QToolButton( );
    m_button->setMaximumSize( 16,16 );
    m_button->setGeometry(-20,-16,16,16);
    m_button->setCheckable( true );

    QFont font = m_button->font();
    font.setFamily("Ubuntu");
    font.setPixelSize(11);
    m_button->setFont( font );

    m_proxy = Circuit::self()->addWidget( m_button );
    m_proxy->setParentItem( this );

    connect( Circuit::self(), SIGNAL( keyEvent( QString, bool ) ),
                        this, SLOT(   keyEvent( QString, bool ) )
                            , Qt::UniqueConnection );
}
SwitchBase::~SwitchBase(){}

void SwitchBase::updateStep()
{
    if( m_changed )
    {
        setSwitch( m_closed );
        m_changed = false;
        update();
}   }

void SwitchBase::setHidden( bool hide, bool hidLabel )
{
    Component::setHidden( hide, hidLabel );

    if( hide ) m_area = QRectF( -8,-2, 16, 4 );
    else       m_area = QRectF( -13,-16*m_numPoles, 26, 16*m_numPoles );
}

void SwitchBase::onbuttonclicked()
{
    m_closed = m_button->isChecked();

    if( m_nClose ) m_closed = !m_closed;
    m_changed = true;
    update();
}

void SwitchBase::setKey( QString key )
{
    if( key.size()>1 ) key = key.left( 1 );
    m_key = key;
    m_button->setText( key );
}

#include "moc_switch_base.cpp"
