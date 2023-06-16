/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "voltsource.h"
#include "itemlibrary.h"
#include "propdialog.h"
#include "simulator.h"
#include "custombutton.h"
#include "iopin.h"

#include "doubleprop.h"

Component* VoltSource::construct( QObject* parent, QString type, QString id )
{ return new VoltSource( parent, type, id ); }

LibraryItem* VoltSource::libraryItem()
{
    return new LibraryItem(
        tr( "Voltage Source" ),
        "Sources",
        "voltsource.png",
        "Voltage Source",
        VoltSource::construct );
}

VoltSource::VoltSource( QObject* parent, QString type, QString id )
          : VarSource( parent, type, id )
{
    m_pin.resize(1);
    m_pin[0] = m_outPin = new IoPin( 0, QPoint(28,16), id+"-outPin", 0, this, source );

    m_outPin->setOutHighV( 0 );
    m_outPin->setOutLowV( 0 );

    m_unit = "V";
    addPropGroup( { tr("Main"), {
new DoubProp<VoltSource>("Value_Volt", tr("Current Value"),"V", this, &VoltSource::getVal,   &VoltSource::setVal ),
new DoubProp<VoltSource>("MaxValue"  , tr("Max. Voltage") ,"V", this, &VoltSource::maxValue, &VoltSource::setMaxValue )
    }, 0} );

    setShowProp("MaxValue");
    setPropStr( "MaxValue", "5 V" );
    dialChanged( 0 );
}
VoltSource::~VoltSource() {}

void VoltSource::updateStep()
{
    if( m_changed ) 
    {
        m_outPin->setOutHighV( m_outValue );
        m_outPin->setOutState( m_button->isChecked() );
        if( m_propDialog ) m_propDialog->updtValues();
        m_changed = false;
}   }
