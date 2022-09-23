/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "gate_and.h"
#include "itemlibrary.h"

#include "intprop.h"
#include "boolprop.h"

Component* AndGate::construct( QObject* parent, QString type, QString id )
{ return new AndGate( parent, type, id ); }

LibraryItem* AndGate::libraryItem()
{
    return new LibraryItem(
        tr( "And Gate" ),
        tr( "Logic/Gates" ),
        "andgate.png",
        "And Gate",
        AndGate::construct );
}

AndGate::AndGate( QObject* parent, QString type, QString id )
       : Gate( parent, type, id, 2 )
{
    addPropGroup( { tr("Main"), {
new IntProp <AndGate>( "Num_Inputs"    , tr("Input Size")   ,"_Inputs", this, &AndGate::numInps,    &AndGate::setNumInps, "uint" ),
new BoolProp<AndGate>( "Inverted"      , tr("Invert Output"),""   , this, &AndGate::invertOuts, &AndGate::setInvertOuts ),
new BoolProp<AndGate>( "Open_Collector", tr("Open Drain")   ,""   , this, &AndGate::openCol,    &AndGate::setOpenCol )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges")   , Gate::edgeProps() } );

    removeProperty(tr("Edges"),"pd_n");
}
AndGate::~AndGate(){}

QPainterPath AndGate::shape() const
{
    int size = (int)m_inPin.size()*4;
    QPainterPath path;
    QVector<QPointF> points;
    points << QPointF( -9,-size )
           << QPointF( -9, size )
           << QPointF(  0, size-2 )
           << QPointF( 10, 8 )
           << QPointF( 10,-8 )
           << QPointF(  0,-size+2 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void AndGate::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawChord( -28, m_area.y(), 37, m_area.height(), -1440/*-16*90*/, 2880/*16*180*/ );
}
