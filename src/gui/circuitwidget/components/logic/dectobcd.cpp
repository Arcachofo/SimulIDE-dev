/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "dectobcd.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "iopin.h"

#include "boolprop.h"

Component* DecToBcd::construct( QObject* parent, QString type, QString id )
{ return new DecToBcd( parent, type, id ); }

LibraryItem* DecToBcd::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("DecToBcd", "Encoder(10/16 to 4)"),
        "Converters",
        "3to2g.png",
        "DecToBcd",
        DecToBcd::construct );
}

DecToBcd::DecToBcd( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 10;

    m_tristate = true;
    m_16Bits = false;
    m_bits = 10;

    QStringList pinList;

    pinList// Outputs:
            << "OR03 A"
            << "OR04 B"
            << "OR05 C"
            << "OR06 D"
            ;
    init( pinList );

    setNumInps( 10,"D", 1 );

    createOePin( "IU03OE ", id+"-in15"); // Output Enable

    addPropGroup( { tr("Main"), {
new BoolProp<DecToBcd>("_16_Bits", tr("16 Bits"),"", this, &DecToBcd::is16Bits, &DecToBcd::set16bits ),
    },groupNoCopy} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({new BoolProp<DecToBcd>( "Invert_Inputs", tr("Invert Inputs"),"", this, &DecToBcd::invertInps, &DecToBcd::setInvertInps,propNoCopy )})
                    +IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps(),0 } );
}
DecToBcd::~DecToBcd(){}

void DecToBcd::stamp()
{
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp();
}

void DecToBcd::voltChanged()
{
    LogicComponent::updateOutEnabled();

    int i;
    for( i=m_bits-2; i>=0; --i ) { if( m_inPin[i]->getInpState() ) break; }
    m_nextOutVal = i+1;
    sheduleOutPuts( this );
}

void DecToBcd::set16bits( bool set )
{
    m_bits =  set ? 16 : 10;
    m_16Bits = set;
    setNumInps( m_16Bits ? 16 : 10, "D", 1 );
    m_oePin->setY( m_area.y()-8 );
}
