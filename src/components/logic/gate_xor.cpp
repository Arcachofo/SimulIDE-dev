/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "gate_xor.h"
#include "itemlibrary.h"

#include "boolprop.h"

#define tr(str) simulideTr("XorGate",str)

Component* XorGate::construct( QString type, QString id )
{ return new XorGate( type, id ); }

LibraryItem* XorGate::libraryItem()
{
    return new LibraryItem(
        tr("Xor Gate" ),
        "Gates",
        "xorgate.png",
        "Xor Gate",
        XorGate::construct );
}

XorGate::XorGate( QString type, QString id )
       : Gate( type, id, 2 )
{
    XorGate::updatePath();

    addPropGroup( { tr("Electric"), IoComponent::inputProps()
        +QList<ComProperty*>({
        new BoolProp<XorGate>("Invert_Inputs", tr("Invert Inputs"),""
                             , this, &XorGate::invertInps, &XorGate::setInvertInps,propNoCopy )
                            })
        + Gate::outputProps()
        + IoComponent::outputType() ,0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
XorGate::~XorGate(){}

bool XorGate::calcOutput( int inputs ) { return (inputs == 1); }

void XorGate::updatePath()
{
    int endY = m_area.height()/2;
    int endX = m_area.width()/2;

    m_path.clear();
    m_path.moveTo(-7,-endY );
    m_path.quadTo( QPoint( endX-2,-endY ), QPoint( endX, 0   ) );
    m_path.quadTo( QPoint( endX-2, endY ), QPoint( -7 , endY ) );
    m_path.quadTo( QPoint( -3  , 0    ), QPoint( -7 ,-endY ) );
}

void XorGate::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    int endY = m_area.height()/2;

    p->drawPath( m_path );

    p->setBrush( Qt::NoBrush );

    QPainterPath curve;
    curve.moveTo(-10, endY );
    curve.quadTo( QPointF(-5.5, 0 ), QPoint(-10,-endY ) );
    p->drawPath( curve );

    Component::paintSelected( p );
}
