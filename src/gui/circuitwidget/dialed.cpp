/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QAbstractSlider>
#include <QGraphicsProxyWidget>

#include "dialed.h"
#include "dialwidget.h"
#include "simulator.h"
#include "circuit.h"

#include "boolprop.h"
#include "doubleprop.h"

Dialed::Dialed( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
{
    m_graphical = true;
    m_area = QRectF( -12, -4.5, 24, 12.5 );

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);

    m_proxy = Circuit::self()->addWidget( &m_dialW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-m_dialW.width()/2, 7) );
    m_slider = false;

    connect( m_dialW.dial(), &QAbstractSlider::valueChanged,
             this,           &Dialed::dialChanged, Qt::UniqueConnection );

    Simulator::self()->addToUpdateList( this );
}
Dialed::~Dialed() {}

QList<ComProperty*> Dialed::dialProps()
{
    return {
new BoolProp<Dialed>( "Slider", tr("Slider"),"", this, &Dialed::slider, &Dialed::setSlider ),
new DoubProp<Dialed>( "Scale" , tr("Scale") ,"", this, &Dialed::scale , &Dialed::setScale )
    };
}

void Dialed::dialChanged( int ) // Called when dial is rotated
{
    m_needUpdate = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Dialed::setLinked( bool l )
{
    Component::setLinked( l );
    /// m_dialW.setVisible( !l );
}

void Dialed::setLinkedValue( int v, int )
{
    m_dialW.setValue( v );
}

void Dialed::setSlider( bool s )
{
    m_slider = s;

    disconnect( m_dialW.dial(), &QAbstractSlider::valueChanged,
                this,           &Dialed::dialChanged );

    m_dialW.setType( s ? 1: 0 );

    connect( m_dialW.dial(), &QAbstractSlider::valueChanged,
             this,           &Dialed::dialChanged, Qt::UniqueConnection );

    updateProxy();
}

