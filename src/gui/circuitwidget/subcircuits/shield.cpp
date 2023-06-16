/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shield.h"
//#include "itemlibrary.h"
//#include "componentselector.h"
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

ShieldSubc::ShieldSubc( QObject* parent, QString type, QString id )
          : BoardSubc( parent, type, id )
{
    m_subcType = Chip::Shield;
    m_attached = false;
    m_boardId = "";
    m_board = NULL;
    setZValue( 1 );

    addPropGroup( {"Hidden", {
new StrProp<ShieldSubc>( "BoardId" , "","", this, &ShieldSubc::boardId, &ShieldSubc::setBoardId )
    }, groupHidden} );
}
ShieldSubc::~ShieldSubc(){}

void ShieldSubc::remove()
{
    if( m_board ) return;
    SubCircuit::remove();
}

void ShieldSubc::setBoard( BoardSubc* board )
{
    if( board )
    {
        m_boardId = board->getUid();
        board->attachShield( this );
    }else{
        m_boardId = "";
        m_board->detachShield( this );
    }
    setParentItem( board );
    m_attached = board  ? true : false;
    m_board = board;
}

void ShieldSubc::connectBoard()
{
    //slotAttach();return;
    m_board = NULL;
    if( m_boardId == "" ) return;

    QString name = Circuit::self()->origId( m_boardId );
    if( name != "" ) m_boardId = name;

    Component* comp = Circuit::self()->getCompById( m_boardId );
    if( comp && comp->itemType() == "Subcircuit" )
    {
        //Circuit::self()->compList()->removeOne( this );
        m_board = static_cast<BoardSubc*>(comp);
        attachToBoard();
        setBoard( m_board );
}   }

void ShieldSubc::slotAttach()
{
    QList<QGraphicsItem*> list = this->collidingItems();
    for( QGraphicsItem* it : list )
    {
        if( it->type() == UserType+1 )    // Component found
        {
            Component* comp =  qgraphicsitem_cast<Component*>( it );
            if( comp->itemType() == "Subcircuit" )
            {
                BoardSubc* board =  (BoardSubc*)comp;
                if( board->subcType() < Board ) continue;

                if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
                Circuit::self()->saveState();

                m_circPos = this->pos();
                m_board = board;
                attachToBoard();
                setBoard( board );
                this->moveTo( m_boardPos );
                break;
}   }   }   }

void ShieldSubc::attachToBoard()
{
    int origX = 8*(m_board->pkgWidth()-m_width)/2;
    m_boardPos = QPointF(origX, 0);
    this->setRotation(0);
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::slotDetach()
{
    if( m_board )
    {
        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        Circuit::self()->saveState();

        this->moveTo( this->scenePos()+QPointF( 8,-8 ) );
        setBoard( NULL );
        renameTunnels();
    }
    m_attached = false;
}

void ShieldSubc::renameTunnels()
{
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
        event->accept();

        if( m_attached )
        {
            QAction* detachAction = menu->addAction( QIcon(":/detach.png"),tr("Detach") );
            connect( detachAction, &QAction::triggered, this, &ShieldSubc::slotDetach );
        }else{
            QAction* attachAction = menu->addAction( QIcon(":/attach.png"),tr("Attach") );
            connect( attachAction, &QAction::triggered, this, &ShieldSubc::slotAttach );
        }

        if( m_board ) m_board->contextMenu( event, menu );
        else          Component::contextMenu( event, menu );
}
