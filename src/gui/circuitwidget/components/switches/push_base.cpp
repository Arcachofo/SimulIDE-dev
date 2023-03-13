/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "push_base.h"
#include "circuit.h"
#include "custombutton.h"

PushBase::PushBase( QObject* parent, QString type, QString id )
        : SwitchBase( parent, type, id )
{
    connect( m_button, &CustomButton::pressed,
                 this, &PushBase::onbuttonPressed, Qt::UniqueConnection );

    connect( m_button, &CustomButton::released,
                 this, &PushBase::onbuttonReleased, Qt::UniqueConnection );
}
PushBase::~PushBase(){}

void PushBase::onbuttonPressed()
{
    m_closed = true;
    if( m_nClose ) m_closed = !m_closed;
    m_button->setChecked( true );
    m_changed = true;
    update();
}

void PushBase::onbuttonReleased()
{
    m_closed = false;
    if( m_nClose ) m_closed = !m_closed;
    m_button->setChecked( false );
    m_changed = true;
    update();
}

void PushBase::keyEvent( QString key, bool pressed )
{
    if( key == m_key )
    {
        if( pressed ) onbuttonPressed();
        else          onbuttonReleased();
}   }
