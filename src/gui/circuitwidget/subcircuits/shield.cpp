/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shield.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "tunnel.h"
#include "node.h"
#include "e-node.h"
//#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"
#include "doubleprop.h"

#define tr(str) simulideTr("ShieldSubc",str)

ShieldSubc::ShieldSubc( QString type, QString id )
          : BoardSubc( type, id )
{
    m_subcType = Chip::Shield;
    m_boardId = "";
    setZValue( 1 );

    addPropGroup( {"Hidden", {
new StrProp<ShieldSubc>( "BoardId", "","", this, &ShieldSubc::boardId, &ShieldSubc::setBoardId )
    }, groupHidden} );
}
ShieldSubc::~ShieldSubc(){}

void ShieldSubc::setBoard( BoardSubc* board )
{
    m_parentBoard = board;
    m_boardId = board->getUid();
    board->attachShield( this );
    if( m_subcType == Shield ) attachToBoard();
    setParentItem( board );
    Circuit::self()->compList()->removeOne( this );
}

void ShieldSubc::connectBoard()
{
    m_parentBoard = NULL;
    if( m_boardId == "" ) return;

    QString name = Circuit::self()->origId( m_boardId );
    if( name != "" ) m_boardId = name;

    Component* comp = Circuit::self()->getCompById( m_boardId );
    if( comp && comp->itemType() == "Subcircuit" )
    {
        m_parentBoard = static_cast<BoardSubc*>(comp);
        setBoard( m_parentBoard );
}   }

void ShieldSubc::slotAttach()
{
    QList<QGraphicsItem*> list = this->collidingItems();
    for( QGraphicsItem* it : list )
    {
        if( it->type() != UserType+1 ) continue;             // Not a Component

        Component* comp = qgraphicsitem_cast<Component*>( it );
        if( !(comp->itemType() == "Subcircuit") ) continue;  // Not a Subcircuit

        BoardSubc* board = (BoardSubc*)comp;
        if( board->subcType() != Board ) continue;           // Not a Board

        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        /// FIXME UNDOREDO: Circuit::self()->saveState();

        setBoard( board );
        break;
    }
}

void ShieldSubc::attachToBoard()
{
    int origX = 8*(m_parentBoard->pkgWidth()-m_width)/2;
    m_boardPos = QPointF(origX, 0);
    this->moveTo( m_boardPos );
    this->setRotation(0);
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::slotDetach()
{
    if( !m_parentBoard ) return;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    /// FIXME UNDO REDO: Circuit::self()->saveState();

    this->moveTo( this->scenePos()+QPointF( 8,-8 ) );
    m_parentBoard->detachShield( this );
    m_boardId = "";
    m_parentBoard = NULL;
    setParentItem( NULL );
    Circuit::self()->compList()->append( this );
    renameTunnels();
}

void ShieldSubc::renameTunnels()
{
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    event->accept();
    QMenu* menu = new QMenu();
    contextMenu( event, menu );
    menu->deleteLater();
}

void ShieldSubc::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    event->accept();

    if( m_parentBoard )
    {
        QAction* detachAction = menu->addAction( QIcon(":/detach.png"),tr("Detach") );
        QObject::connect( detachAction, &QAction::triggered, [=](){ slotDetach(); } );
        m_parentBoard->contextMenu( event, menu );
    }else{
        QAction* attachAction = menu->addAction( QIcon(":/attach.png"),tr("Attach") );
        QObject::connect( attachAction, &QAction::triggered, [=](){ slotAttach(); } );
        SubCircuit::contextMenu( event, menu );
    }
}
