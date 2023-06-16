/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "buffer.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

Component* Buffer::construct( QObject* parent, QString type, QString id )
{ return new Buffer( parent, type, id ); }

LibraryItem* Buffer::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("Buffer", "Buffer"),
        "Gates",
        "buffer.png",
        "Buffer",
        Buffer::construct );
}

Buffer::Buffer( QObject* parent, QString type, QString id )
      : Gate( parent, type, id, 1 )
{
    m_area = QRect(-8, -8, 16, 16 );
    
    setOePin( new IoPin( 90, QPoint( 0,-12 ), m_id+"-Pin_outEnable", 0, this, input ) );
    Buffer::setTristate( false );

    addPropGroup( { tr("Main"), {

    },0} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({
new BoolProp<Buffer>("Invert_Inputs", tr("Invert Inputs"),"", this, &Buffer::invertInps, &Buffer::setInvertInps, propNoCopy )
                    })
                    +IoComponent::outputProps()+IoComponent::outputType()
+QList<ComProperty*>({
new BoolProp<Buffer>("Tristate", tr("Tristate"),"", this, &Buffer::tristate, &Buffer::setTristate, propNoCopy )
                    })
    ,0} );
    addPropGroup( { tr("Edges") , Gate::edgeProps(),0 } );

    removeProperty("pd_n");
}
Buffer::~Buffer(){}

void Buffer::setTristate( bool t )  // Activate or deactivate OE Pin
{
    LogicComponent::setTristate( t );
    m_oePin->setLabelText( "" );
}

QPainterPath Buffer::shape() const
{
    QPainterPath path;
    QVector<QPointF> points;
    
    points << QPointF(-9,-9 )
           << QPointF(-9, 9 )
           << QPointF( 9, 1 )
           << QPointF( 9,-1 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Buffer::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    static const QPointF points[4] = {
        QPointF(-8,-8 ),
        QPointF(-8, 8 ),
        QPointF( 8, 1 ),
        QPointF( 8,-1 )             };

    p->drawPolygon( points, 4 );
}
