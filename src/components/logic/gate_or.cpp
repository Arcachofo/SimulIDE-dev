/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "gate_or.h"
#include "itemlibrary.h"

#include "intprop.h"
#include "boolprop.h"

#define tr(str) simulideTr("OrGate",str)

Component* OrGate::construct( QString type, QString id )
{ return new OrGate( type, id ); }

LibraryItem* OrGate::libraryItem()
{
    return new LibraryItem(
        tr("Or Gate"),
        "Gates",
        "orgate.png",
        "Or Gate",
        OrGate::construct );
}

OrGate::OrGate( QString type, QString id )
      : Gate( type, id, 2 )
{
    OrGate::updatePath();

    addPropGroup( { tr("Electric"),
        IoComponent::inputProps()
        +QList<ComProperty*>({
        new BoolProp<OrGate>("Invert_Inputs", tr("Invert Inputs"),""
                            , this, &OrGate::invertInps, &OrGate::setInvertInps, propNoCopy ),

        new IntProp <OrGate>("Num_Inputs", tr("Input Size"),"_Inputs"
                            , this, &OrGate::numInps, &OrGate::setNumInputs, propNoCopy,"uint" )
                    })
        + Gate::outputProps()
        + IoComponent::outputType() ,0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
OrGate::~OrGate(){}

bool OrGate::calcOutput( int inputs ) { return (inputs > 0); }

void OrGate::updatePath()
{
    int endY = m_area.height()/2;
    int endX = m_area.width()/2;

    m_path = QPainterPath();
    m_path.moveTo(-10,-endY );
    m_path.quadTo( QPoint( endX-2,-endY ), QPoint( endX, 0    ) );
    m_path.quadTo( QPoint( endX-2, endY ), QPoint( -10 , endY ) );
    m_path.quadTo( QPoint(     -6, 0    ), QPoint( -10 ,-endY ) );
}
