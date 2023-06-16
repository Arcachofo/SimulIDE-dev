/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "adc.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* ADC::construct( QObject* parent, QString type, QString id )
{ return new ADC( parent, type, id ); }

LibraryItem* ADC::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("ADC", "ADC"),
        "Other Logic",
        "1to3.png",
        "ADC",
        ADC::construct );
}

ADC::ADC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 9;

    setLabelPos(-16,-80, 0);
    setNumOutputs( 8 );    // Create Output Pins
    m_maxVolt = 5;

    addPropGroup( { tr("Main"), {
new IntProp <ADC>("Num_Bits", tr("Size")         ,"_Bits", this, &ADC::numOuts, &ADC::setNumOutputs, 0,"uint" ),
new DoubProp<ADC>("Vref"    , tr("Reference Voltage"),"V", this, &ADC::maxVolt, &ADC::setMaxVolt ),
    },groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges")   , IoComponent::edgeProps()                            ,0 } );
}
ADC::~ADC(){}

void ADC::stamp()
{
    m_inPin[0]->changeCallBack( this );
    LogicComponent::stamp();
}

void ADC::voltChanged()
{
    double volt = m_inPin[0]->getVoltage();
    m_nextOutVal = volt*m_maxValue/m_maxVolt+0.1;
    LogicComponent::sheduleOutPuts( this );
}

void ADC::setNumOutputs( int outs )
{
    if( outs < 1 ) return;
    m_maxValue = pow( 2, outs )-1;
    IoComponent::setNumOuts( outs, "D" );
    IoComponent::setNumInps( 1, "In" );
}
