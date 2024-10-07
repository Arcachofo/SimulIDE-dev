/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "module.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "tunnel.h"

#include "doubleprop.h"

#define tr(str) simulideTr("ModuleSubc",str)

ModuleSubc::ModuleSubc( QString type, QString id )
          : ShieldSubc( type, id )
{
    m_subcType = "Module";
    setZValue( 2 );

    addPropGroup( { tr("Main"), {
new DoubProp<ModuleSubc>( "Z_Value", tr("Z Value"),"", this, &ModuleSubc::zVal, &ModuleSubc::setZVal )
    },0} );
}
ModuleSubc::~ModuleSubc(){}

void ModuleSubc::setZVal( double v )
{
    if     ( v < 2 ) v = 2;
    else if( v > 9 ) v = 9;
    setZValue( v );
}

void ModuleSubc::slotAttach()
{
    double myZ = this->zValue();

    QList<QGraphicsItem*> list = this->collidingItems();
    QList<BoardSubc*> boardList;
    for( QGraphicsItem* it : list )
    {
        if( it->type() != UserType+1 ) continue;             // Not a Component

        Component* comp = qgraphicsitem_cast<Component*>( it );
        if( !(comp->itemType() == "Subcircuit") ) continue;  // Not a Subcircuit

        BoardSubc* board = (BoardSubc*)comp;
        if( !board->isBoard() ) continue;            // Not a Board

        QString subcType = board->subcTypeStr();
        if( subcType == "Shield" || subcType == "Module" ){
            ShieldSubc* shield = static_cast<ShieldSubc*>(board);
            if( m_shields.contains( shield ) ) continue;     // Is my child
        }
        boardList.append( board );
    }
    for( BoardSubc* board : boardList )
    {
        if( board->zValue() > myZ ) continue;                            // Is above me

        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        /// FIXME UNDOREDO: Circuit::self()->saveState();

        setBoard( board );
        m_boardPos = pos() - board->pos();
        while( board->parentBoard() ){
            board = board->parentBoard();
            m_boardPos -= board->pos();
        }
        this->moveTo( m_boardPos );
        break;
    }
}

void ModuleSubc::renameTunnels()
{
    for( Tunnel* tunnel : getPinTunnels() ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
    for( Tunnel* tunnel : m_subcTunnels   ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}
