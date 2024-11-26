/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "buffer.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

#define tr(str) simulideTr("Buffer",str)

Component* Buffer::construct( QString type, QString id )
{ return new Buffer( type, id ); }

LibraryItem* Buffer::libraryItem()
{
    return new LibraryItem(
        tr("Buffer"),
        "Gates",
        "buffer.png",
        "Buffer",
        Buffer::construct );
}

Buffer::Buffer( QString type, QString id )
      : Gate( type, id, 1 )
{
    m_area = QRect(-8, -8, 16, 16 );
    Buffer::updatePath();
    
    setOePin( new IoPin( 90, QPoint( 0,-12 ), m_id+"-Pin_outEnable", 0, this, input ) );
    Buffer::setTristate( false );

    addPropGroup( { tr("Main"), {

    },0} );
    addPropGroup( { tr("Electric"),
        IoComponent::inputProps()
        +QList<ComProperty*>({
        new BoolProp<Buffer>("Invert_Inputs", tr("Invert Inputs"),""
                            , this, &Buffer::invertInps, &Buffer::setInvertInps, propNoCopy )
                            })
        +Gate::outputProps()
        +IoComponent::outputType()
        +QList<ComProperty*>({
        new BoolProp<Buffer>("Tristate", tr("Tristate"),""
                            , this, &Buffer::tristate, &Buffer::setTristate, propNoCopy )
                            })
    ,0} );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
Buffer::~Buffer(){}

void Buffer::setTristate( bool t )  // Activate or deactivate OE Pin
{
    LogicComponent::setTristate( t );
    m_oePin->setLabelText( "" );
}

void Buffer::updatePath()
{
    m_path = QPainterPath();
    m_path.moveTo(-8,-8 );
    m_path.lineTo(-8, 8 );
    m_path.lineTo( 8, 1);
    m_path.lineTo( 8,-1 );
    m_path.lineTo(-8,-8 );
}

