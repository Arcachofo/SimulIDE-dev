/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "bcdto7s.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

Component* BcdTo7S::construct( QObject* parent, QString type, QString id )
{ return new BcdTo7S( parent, type, id ); }

LibraryItem* BcdTo7S::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("BcdTo7S", "Bcd To 7S."),
        "Converters",
        "2to3g.png",
        "BcdTo7S",
        BcdTo7S::construct );
}

BcdTo7S::BcdTo7S( QObject* parent, QString type, QString id )
       : BcdBase( parent, type, id )
{
    m_width  = 4;
    m_height = 8;

    init({         // Inputs:
            "IL03S0",
            "IL04S1",
            "IL05S2",
            "IL06S3",
                    // Outputs:
            "OR01a",
            "OR02b",
            "OR03c",
            "OR04d",
            "OR05e",
            "OR06f",
            "OR07g"
        });

    createOePin( "IU01OE ", id+"-in4");

    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({new BoolProp<BcdTo7S>( "Invert_Inputs", tr("Invert Inputs"),"", this, &BcdTo7S::invertInps, &BcdTo7S::setInvertInps, propNoCopy )})
                    +IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps(),0 } );
}
BcdTo7S::~BcdTo7S(){}

void BcdTo7S::stamp()
{
    BcdBase::stamp();

    uint8_t value = m_values[0];
    for( int i=0; i<7; ++i ) m_outPin[i]->setOutState( value & (1<<i) );
}

void BcdTo7S::voltChanged()
{
    LogicComponent::updateOutEnabled();
    BcdBase::voltChanged();
    m_nextOutVal = m_digit;
    sheduleOutPuts( this );
}
