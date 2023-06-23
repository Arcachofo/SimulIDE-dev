/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QGraphicsProxyWidget>

#include "switch_base.h"
#include "simulator.h"
#include "circuit.h"
#include "label.h"
#include "custombutton.h"

SwitchBase::SwitchBase( QObject* parent, QString type, QString id )
          : MechContact( parent, type, id )
{
    m_graphical = true;
    m_changed = true;
    m_key = "";

    m_area =  QRectF( 0,0,0,0 );
    m_ePin.resize(2);
    m_idLabel->setPos(-12,-24);

    m_button = new CustomButton( );
    m_button->setMaximumSize( 16, 16 );
    m_button->setCheckable( true );

    m_proxy = Circuit::self()->addWidget( m_button );
    m_proxy->setParentItem( this );
    m_proxy->setPos(-20,-16 );
    m_proxy->setTransformOriginPoint( m_proxy->boundingRect().center() );

    connect( Circuit::self(), &Circuit::keyEvent,
                        this, &SwitchBase::keyEvent, Qt::UniqueConnection );
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

void SwitchBase::rotateAngle( double a )
{
    Component::rotateAngle( a );
    rotateText(-a );
}

void SwitchBase::rotateText( double angle )
{
    double rot = m_proxy->rotation();
    m_proxy->setRotation( rot + angle );
}

void SwitchBase::setHidden( bool hide, bool hidArea, bool hidLabel )
{
    Component::setHidden( hide, hidArea, hidLabel );
    if  ( hidArea ) m_area = QRectF( 0, 0,-4,-4 );
    else if( hide ) m_area = QRectF(-6, 6, 12, 12 );
    else            m_area = QRectF(-13,-16*m_numPoles, 26, 16*m_numPoles );

    m_proxy->setFlag( QGraphicsItem::ItemStacksBehindParent, hide && !hidArea );
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
