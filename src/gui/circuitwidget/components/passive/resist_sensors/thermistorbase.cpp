/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "thermistorbase.h"

#include "doubleprop.h"

ThermistorBase::ThermistorBase( QObject* parent, QString type, QString id )
              : VarResBase( parent, type, id  )
{
    setValLabelPos(-16, 30, 0);

    m_ptc  = false;

    ThermistorBase::setMaxVal( 200 );
    setVal( 25 );

    addPropGroup( { tr("Main"), {
new DoubProp<ThermistorBase>( "Temp"     , tr("Current Value"),"ºC", this, &ThermistorBase::getVal,  &ThermistorBase::setVal ),
new DoubProp<ThermistorBase>( "Min_Temp" , tr("Minimum Value"),"ºC", this, &ThermistorBase::minVal,  &ThermistorBase::setMinVal ),
new DoubProp<ThermistorBase>( "Max_Temp" , tr("Maximum Value"),"ºC", this, &ThermistorBase::maxVal,  &ThermistorBase::setMaxVal ),
new DoubProp<ThermistorBase>( "Dial_Step", tr("Dial Step")    ,"ºC", this, &ThermistorBase::getStep, &ThermistorBase::setStep )
    }, 0} );
}
ThermistorBase::~ThermistorBase(){}

void ThermistorBase::setPtc( bool ptc )
{
    return; // TODO
    m_ptc = ptc;
    m_needUpdate = true;
}

void ThermistorBase::setMinVal( double min )
{
    if( min > m_maxVal ) min = m_maxVal;
    m_minVal = min;

    updtValue();
}

void ThermistorBase::setMaxVal( double max )
{
    if( max < m_minVal ) max = m_minVal;
    m_maxVal = max;

    updtValue();
}

