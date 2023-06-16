/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

// Capacitor model using backward euler approximation
// consists of a current source in parallel with a resistor.

#include "capacitorbase.h"
#include "simulator.h"
#include "pin.h"

#include "doubleprop.h"
#include "intprop.h"

CapacitorBase::CapacitorBase( QObject* parent, QString type, QString id )
             : Reactive( parent, type, id )
{
    m_area = QRectF( -10, -10, 20, 20 );

    m_pin[0]->setLength( 12 );
    m_pin[1]->setLength( 12 );

    m_value = 0.00001; // Farads

    addPropGroup( { tr("Main"), {
new DoubProp<CapacitorBase>( "Capacitance", tr("Capacitance")    , "F"    , this, &CapacitorBase::value   , &CapacitorBase::setValue ),
new DoubProp<CapacitorBase>( "Resistance" , tr("Resistance")      ,"Ω"    , this, &CapacitorBase::resist  , &CapacitorBase::setResist ),
new DoubProp<CapacitorBase>( "InitVolt"   , tr("Initial Voltage"), "V"    , this, &CapacitorBase::initVolt, &CapacitorBase::setInitVolt ),
new IntProp <CapacitorBase>( "AutoStep"   , tr("Auto Step")      ,"_Steps", this, &CapacitorBase::autoStep, &CapacitorBase::setAutoStep,0,"uint" )
    },0 } );

    setShowProp("Capacitance");
    setPropStr( "Capacitance", "10 µF" );
}
CapacitorBase::~CapacitorBase(){}
