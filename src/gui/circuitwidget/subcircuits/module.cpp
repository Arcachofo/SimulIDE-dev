/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "module.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "circuit.h"

#include "doubleprop.h"

ModuleSubc::ModuleSubc( QObject* parent, QString type, QString id )
          : ShieldSubc( parent, type, id )
{
    //m_subcType = Chip::Module;

    addPropGroup( { tr("Main"), {
new DoubProp<ModuleSubc>( "Z_Value", tr("Z Value"),"", this, &ModuleSubc::zVal, &ModuleSubc::setZVal )
    }} );
}
ModuleSubc::~ModuleSubc(){}

void ModuleSubc::stamp()
{
    QList<QGraphicsItem*> list = this->collidingItems();
    //std::vector<Pin*> connPins = m_connPins;
    //m_connPins.clear();
    //m_connPins.resize( m_size, NULL );
    for( QGraphicsItem* it : list )
    {
        if( it->type() == 65536+3 )        // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );
            if( pin->parentItem() == this ) continue;
            if( pin->connector() ) continue; // Pin is already connected
            if( pin->isBus() )     continue; // Don't connect to Bus

            QPointF pinPos = this->mapFromScene( pin->scenePos() );
            int y = pinPos.y()+24;
            int x = abs( pinPos.x() );
            //qDebug() <<pin->pinId()<<pinPos<<y;

            if( (x < 1) && (y%8 < 2) )
            {
                int i = y/8;
                eNode* node = m_pin[i]->getEnode();
                pin->setEnode( node );
            }
        }
    }
}

void ModuleSubc::setZVal( double v )
{
    if     ( v < 1 ) v = 1;
    else if( v > 9 ) v = 9;
    setZValue( v );
}

void ModuleSubc::slotAttach()
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
                if( !(board->subcType() == Board) ) continue;

                if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
                Circuit::self()->saveState();

                m_board = board;
                m_boardId = m_board->getUid();
                m_board->attachShield( this );

                m_circPos = this->pos();

                //int origX = 8*(m_board->pkgWidth()-m_width)/2;
                this->setParentItem( m_board );
                //this->moveTo( QPointF(origX, 0) );
                //this->setRotation(0);

                //for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->tunnelUid() );
                m_attached = true;
                break;
}   }   }   }

void ModuleSubc::slotDetach()
{
    if( m_board )
    {
        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        Circuit::self()->saveState();

        m_board->detachShield( this );
        //this->moveTo( m_circPos );
        this->setParentItem( NULL );
        //for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
        m_board = NULL;
    }
    m_attached = false;
}

#include "moc_module.cpp"
