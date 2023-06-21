/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>
#include <QAbstractSlider>
#include <QGraphicsProxyWidget>

#include "varresbase.h"
#include "propdialog.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"
#include "label.h"

VarResBase::VarResBase( QObject* parent, QString type, QString id )
          : Dialed( parent, type, id )
          , eResistor( id )
{
    m_area = QRectF(-11,-11, 22, 16 );
    m_graphical = true;

    m_pin.resize(2);
    m_ePin[0] = m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_ePin[1] = m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);

    m_pin[0]->setLength( 5 );
    m_pin[1]->setLength( 5 );

    m_idLabel->setPos(-12,-24);
    setLabelPos(-16,-24, 0);
    setValLabelPos(-16, 30, 0);

    m_minVal = 0;
    m_maxVal = 1000;
    m_step = 0;

    Simulator::self()->addToUpdateList( this );
}
VarResBase::~VarResBase(){}

void VarResBase::initialize()
{
    m_needUpdate = true;
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

    m_needUpdate = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void VarResBase::updtValue()
{
    if     ( m_value > m_maxVal ) m_value = m_maxVal;
    else if( m_value < m_minVal ) m_value = m_minVal;

    double dialV = (m_value-m_minVal)*1000/(m_maxVal-m_minVal);
    m_dialW.setValue( dialV );

    if( m_propDialog ) m_propDialog->updtValues();
}

void VarResBase::updateProxy()
{
    m_proxy->setPos( QPoint(-m_dialW.width()/2, 7) );
}
