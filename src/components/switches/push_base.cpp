/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "push_base.h"
#include "circuit.h"
#include "custombutton.h"

PushBase::PushBase( QString type, QString id )
        : SwitchBase( type, id )
{
    QObject::connect( m_button, &CustomButton::pressed , [=](){ onbuttonPressed(); } );
    QObject::connect( m_button, &CustomButton::released, [=](){ onbuttonReleased(); } );
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
    if( key.toUpper() == m_key )
    {
        if( pressed ) onbuttonPressed();
        else          onbuttonReleased();
}   }
