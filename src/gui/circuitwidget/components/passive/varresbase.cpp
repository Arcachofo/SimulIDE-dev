/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "varresbase.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"

static const char* VarResBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Max Resistance"),
    QT_TRANSLATE_NOOP("App::Property","Min Resistance")
};


VarResBase::VarResBase( QObject* parent, QString type, QString id )
            : Component( parent, type, id )
            , eResistor( id )
{
    Q_UNUSED( VarResBase_properties );

    m_graphical = true;

    m_area = QRectF( -11, -11, 22, 16 );
    
    m_pin.resize( 2 );
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-8-8,0);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-rPin"));
    pinPos = QPoint(8+8,0);
    m_pin[1] = new Pin( 0, pinPos, pinId, 1, this);
    m_ePin[1] = m_pin[1];

    m_dialW.setupWidget();
    m_dialW.setFixedSize( 24, 24 );

    m_dial = m_dialW.dial;
    m_dial->setMinimum(0);
    m_dial->setMaximum(1000);
    m_dial->setSingleStep(25);

    m_proxy = Circuit::self()->addWidget( &m_dialW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint( -12, 7) );

    m_idLabel->setPos(-12,-24);
    setLabelPos(-16,-24, 0);

    setValLabelPos(-16, 30, 0);
    setShowVal( true );

    m_dial->setMinimum(0);
    m_dial->setMaximum(999);

    m_step = 1;

    Simulator::self()->addToUpdateList( this );

    connect( m_dial, SIGNAL(valueChanged(int)),
             this,   SLOT  (dialChanged(int)), Qt::UniqueConnection );
}
VarResBase::~VarResBase(){}

void VarResBase::initialize()
{
    m_changed = true;
}

void VarResBase::dialChanged( int val )
{
    if( m_step > 0 ) val = round(val/m_step)*m_step;
    Component::setValue( val/m_unitMult ); // Takes care about units multiplier

    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

int VarResBase::getVal()
{
    return m_dial->value();
}

void VarResBase::setVal( int val )
{
    m_dial->setValue( val );
}

#include "moc_varresbase.cpp"
