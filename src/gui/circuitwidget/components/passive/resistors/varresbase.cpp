/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>
#include <QDial>
#include <QGraphicsProxyWidget>

#include "varresbase.h"
#include "propdialog.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"
#include "label.h"

VarResBase::VarResBase( QObject* parent, QString type, QString id )
            : Comp2Pin( parent, type, id )
            , eResistor( id )
{
    m_area = QRectF( -11, -11, 22, 16 );
    m_graphical = true;

    m_ePin[0] = m_pin[0];
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

    m_minVal = 0;
    m_maxVal = 1000;
    m_step = 0;

    Simulator::self()->addToUpdateList( this );

    connect( m_dial, SIGNAL(valueChanged(int)),
             this,   SLOT  (dialChanged(int)), Qt::UniqueConnection );
}
VarResBase::~VarResBase(){}

void VarResBase::initialize()
{
    m_changed = true;
}

void VarResBase::setMinVal( double min )
{
    if( min < 1e-12    ) min = 1e-12;
    if( min > m_maxVal ) min = m_maxVal;
    m_minVal = min;

    updtValue();
}

void VarResBase::setMaxVal( double max )
{
    if( max < 1e-12    ) max = 1e-12;
    if( max < m_minVal ) max = m_minVal;
    m_maxVal = max;

    updtValue();
}

void VarResBase::setVal( double val )
{
    m_value = val;
    updtValue();
}

void VarResBase::dialChanged( int val )
{
    m_value = m_minVal+val*( m_maxVal-m_minVal)/1000;
    if( m_step > 0 ) m_value = round( m_value/m_step )*m_step;

    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void VarResBase::updtValue()
{
    if     ( m_value > m_maxVal ) m_value = m_maxVal;
    else if( m_value < m_minVal ) m_value = m_minVal;

    double dialV = (m_value-m_minVal)*1000/(m_maxVal-m_minVal);
    m_dial->setValue( dialV );

    if( m_propDialog ) m_propDialog->updtValues();
}

#include "moc_varresbase.cpp"
