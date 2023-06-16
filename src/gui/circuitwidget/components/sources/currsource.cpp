/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "currsource.h"
#include "itemlibrary.h"
#include "propdialog.h"
#include "simulator.h"
#include "custombutton.h"
#include "pin.h"

#include "doubleprop.h"

Component* CurrSource::construct( QObject* parent, QString type, QString id )
{ return new CurrSource( parent, type, id ); }

LibraryItem* CurrSource::libraryItem()
{
    return new LibraryItem(
        tr( "Current Source" ),
        "Sources",
        "cursource.png",
        "Current Source",
        CurrSource::construct );
}

CurrSource::CurrSource( QObject* parent, QString type, QString id )
          : VarSource( parent, type, id )
{
    m_pin.resize(1);
    m_pin[0] = m_outPin = new Pin( 0, QPoint(28,16), id+"-outPin", 0, this );

    m_unit = "A";

    addPropGroup( { tr("Main"), {
new DoubProp<CurrSource>( "Value_Amp",tr("Current Value"),"A", this, &CurrSource::getVal,   &CurrSource::setVal ),
new DoubProp<CurrSource>( "MaxValue" ,tr("Max. Current") ,"A", this, &CurrSource::maxValue, &CurrSource::setMaxValue )
    }, 0} );

    setShowProp("MaxValue");
    setPropStr( "MaxValue", "1 A" );
    dialChanged( 0 );
}
CurrSource::~CurrSource(){}

void CurrSource::stamp()
{
    m_outPin->createCurrent();
}
void CurrSource::updateStep()
{
    if( m_changed ) 
    {
        double current = m_button->isChecked() ? m_outValue : 0;
        m_outPin->stampCurrent( current );
        if( m_propDialog ) m_propDialog->updtValues();
        m_changed = false;
}   }

