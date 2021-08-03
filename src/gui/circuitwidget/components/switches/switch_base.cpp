/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "switch_base.h"
#include "simulator.h"
#include "circuit.h"

static const char* SwitchBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Key")
};


SwitchBase::SwitchBase( QObject* parent, QString type, QString id )
          : MechContact( parent, type, id )
{
    Q_UNUSED( SwitchBase_properties );

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
    }
}

void SwitchBase::setHidden( bool hide, bool hidLabel )
{
    Component::setHidden( hide, hidLabel );

    if( hide ) m_area = QRectF( -8,-2, 16, 4 );
    else       m_area = QRectF( -13,-16*m_numPoles, 26, 16*m_numPoles );
}

void SwitchBase::onbuttonclicked()
{
    m_closed = false;
    if( m_button->isChecked() ) m_closed = true;
    if( m_nClose )              m_closed = !m_closed;
    m_changed = true;
    
    update();
}

QString SwitchBase::key()
{
    return m_key;
}

void SwitchBase::setKey( QString key )
{
    if( key.size()>1 ) key = key.left( 1 );
    m_key = key;
    //m_shortcut->setKey( key );
    m_button->setText( key );
}


void SwitchBase::remove()
{
    Simulator::self()->remFromUpdateList( this );
    MechContact::remove();
}

#include "moc_switch_base.cpp"
