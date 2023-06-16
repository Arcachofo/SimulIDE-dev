/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "logicsubc.h"
#include "circuit.h"

#include "doubleprop.h"

LogicSubc::LogicSubc( QObject* parent, QString type, QString id )
         : SubCircuit( parent, type, id )
{
    m_subcType = Chip::Logic;

    m_inHighV = 2.5;
    m_inLowV  = 2.5;
    m_ouHighV = 5;
    m_ouLowV  = 0;
    m_inImp = 1e9;
    m_ouImp = 40;
    m_propDelay = 10*1000; // 10 ns
    m_timeLH = 3000;
    m_timeHL = 4000;

    addPropGroup( { tr("Electric"), {
    new ComProperty( "", tr("Inputs:"),"","",0),
    new DoubProp<LogicSubc>( "Input_High_V", tr("Low to High Threshold"),"V", this, &LogicSubc::inputHighV, &LogicSubc::setInputHighV ),
    new DoubProp<LogicSubc>( "Input_Low_V" , tr("High to Low Threshold"),"V", this, &LogicSubc::inputLowV,  &LogicSubc::setInputLowV ),
    new DoubProp<LogicSubc>( "Input_Imped" , tr("Input Impedance")      ,"Ω", this, &LogicSubc::inputImp,   &LogicSubc::setInputImp ),
    new ComProperty( "", tr("Outputs:"),"","",0),
    new DoubProp<LogicSubc>( "Out_High_V", tr("Output High Voltage"),"V", this, &LogicSubc::outHighV, &LogicSubc::setOutHighV ),
    new DoubProp<LogicSubc>( "Out_Low_V" , tr("Output Low Voltage") ,"V", this, &LogicSubc::outLowV,  &LogicSubc::setOutLowV ),
    new DoubProp<LogicSubc>( "Out_Imped" , tr("Output Impedance")   ,"Ω", this, &LogicSubc::outImp,  &LogicSubc::setOutImp )
    } } );
    addPropGroup( { tr("Edges"), {
    new DoubProp<LogicSubc>( "Tpd_ps", tr("Gate Delay "),"ps", this, &LogicSubc::propDelay, &LogicSubc::setPropDelay ),
    new DoubProp<LogicSubc>( "Tr_ps" , tr("Rise Time")  ,"ps", this, &LogicSubc::riseTime,  &LogicSubc::setRiseTime ),
    new DoubProp<LogicSubc>( "Tf_ps" , tr("Fall Time")  ,"ps", this, &LogicSubc::fallTime,  &LogicSubc::setFallTime )
    } } );
}
LogicSubc::~LogicSubc(){}

void LogicSubc::setInputHighV( double volt )
{
    m_inHighV = volt;
    for( Component* c : m_compList )
        c->setPropStr("Input_High_V", QString::number(volt)+" V");
}

void LogicSubc::setInputLowV( double volt )
{
    m_inLowV = volt;
    for( Component* c : m_compList )
        c->setPropStr("Input_Low_V", QString::number(volt)+" V");
}

void LogicSubc::setOutHighV( double volt )
{
    m_ouHighV = volt;
    for( Component* c : m_compList )
        c->setPropStr("Out_High_V", QString::number(volt)+" V");
}

void LogicSubc::setOutLowV( double volt )
{
    m_ouLowV = volt;
    for( Component* c : m_compList )
        c->setPropStr("Out_Low_V", QString::number(volt)+" V");
}

void LogicSubc::setInputImp( double imp )
{
    m_inImp = imp;
    for( Component* c : m_compList )
        c->setPropStr("Input_Imped", QString::number(imp)+" Ω");
}

void LogicSubc::setOutImp( double imp )
{
    m_ouImp = imp;
    for( Component* c : m_compList )
        c->setPropStr("Out_Imped", QString::number(imp)+" Ω");
}

void LogicSubc::setPropDelay( double pd )
{
    m_propDelay = pd*1e12;
    for( Component* c : m_compList )
        c->setPropStr("Tpd_ps", QString::number(m_propDelay)+" ps");
}

void LogicSubc::setRiseTime( double time )
{
    m_timeLH = time*1e12;
    for( Component* c : m_compList )
        c->setPropStr("Tr_ps", QString::number(m_timeLH)+" ps");
}

void LogicSubc::setFallTime( double time )
{
    m_timeHL = time*1e12;
    for( Component* c : m_compList )
        c->setPropStr("Tf_ps", QString::number(m_timeHL)+" ps");
}
