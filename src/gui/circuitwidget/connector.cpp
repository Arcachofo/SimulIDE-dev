/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "connector.h"
#include "connectorline.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"
#include "e-node.h"
#include "utils.h"

#include "stringprop.h"

Connector::Connector( QObject* parent, QString type, QString id, Pin* startpin, Pin* endpin )
         : CompBase( parent, type, id )
{
    if( id.isEmpty() )
        qDebug() << "ERROR! Connector::Connector empty Id";
    m_actLine   = 0;
    m_lastindex = 0;
    
    m_isBus = false;
    m_freeLine = false;

    if( startpin ){
        m_startPin   = startpin;
        m_startpinid = startpin->objectName();
        if( m_startPin->isBus() ) setIsBus( true );
    }
    if( endpin ){
        m_endPin   = endpin;
        m_endpinid = endpin->objectName();
        m_startPin->setConnector( this );
        m_endPin->setConnector( this );
        m_startPin->setConPin( m_endPin );
        m_endPin->setConPin( m_startPin );
        if( m_isBus ) m_endPin->setIsBus( true );
    }else{
        m_endPin   = NULL;
        m_endpinid = "";
    }

    addPropGroup( {"Main", {
new StringProp<Connector>( "itemtype"  ,"","", this, &Connector::itemType,   &Connector::dummySetter ),
new StringProp<Connector>( "uid"       ,"","", this, &Connector::getUid,     &Connector::dummySetter ),
new StringProp<Connector>( "startpinid","","", this, &Connector::startPinId, &Connector::dummySetter ),
new StringProp<Connector>( "endpinid"  ,"","", this, &Connector::endPinId,   &Connector::dummySetter ),
new StringProp<Connector>( "enodeid"   ,"","", this, &Connector::enodId,     &Connector::dummySetter ),
new StringProp<Connector>( "pointList" ,"","", this, &Connector::pListStr,   &Connector::dummySetter ),
    }} );
}
Connector::~Connector(){}

void Connector::remNullLines()      // Remove lines with leght = 0 or aligned
{
    if( m_conLineList.length() < 2 ) 
    {
        refreshPointList();
        return;
    }
    for( ConnectorLine* line : m_conLineList )
    {
        if( line->isDiagonal() ) continue;
        
        int index = m_conLineList.indexOf( line );
        if( index < m_conLineList.length()-1 )      //
        {
            ConnectorLine* line2 = m_conLineList.at( index+1 );
            
            if( line2->isDiagonal() ) continue;

            if( line->dx() == line2->dx() || line->dy() == line2->dy() ) // Lines aligned or null line
            {
                line2->sSetP1( line->p1() );
                remConLine( line  );
    }   }   }
    if( m_conLineList.length() < 2 )
    {
        m_lastindex = 0;
        m_actLine   = 0;
    }
    refreshPointList();
}

void Connector::remConLine( ConnectorLine* line  )
{
    int index = m_conLineList.indexOf( line );

    connectLines( index-1, index+1 );
    if( line->scene() ) Circuit::self()->removeItem( line );
    m_conLineList.removeOne( line );
    if( m_actLine > 0 )  m_actLine -= 1;
}

void Connector::setPointList( QStringList plist )
{
    remLines();

    m_pointList = plist;

    int p1x = plist.first().toInt();
    int p1y = plist.at(1).toInt();
    int p2x = plist.at(plist.size()-2).toInt();
    int p2y = plist.last().toInt();

    addConLine( m_startPin->scenePos().x(), m_startPin->scenePos().y(), p1x, p1y, 0 );

    for (int i=2; i<plist.size(); i+=2)
    {
        p2x = plist.at(i).toInt();
        p2y = plist.at(i+1).toInt();
        addConLine( p1x, p1y, p2x, p2y, i/2 );
        p1x = p2x;
        p1y = p2y;
}   }

void Connector::refreshPointList()
{
    if( m_conLineList.isEmpty() ) return;

    QStringList list;
    QString     data;

    data.setNum( m_conLineList.at(0)->p1().x() );
    list.append( data );
    data.setNum( m_conLineList.at(0)->p1().y() );
    list.append( data );
    int count = m_conLineList.size();
    for( int i=0; i<count; i++ )
    {
        data.setNum( m_conLineList.at(i)->p2().x() );
        list.append( data );
        data.setNum( m_conLineList.at(i)->p2().y() );
        list.append( data );
    }
    m_pointList = list;
}

void Connector::addConLine( ConnectorLine* line, int index )
{
    if( index > 0  && index < m_conLineList.size() ) disconnectLines( index-1, index );

    m_conLineList.insert( index, line );
    Circuit::self()->addItem(line);

    if( index > 0 )
    {
        connectLines( index-1, index );
        m_conLineList.at( index-1 )->sSetP2( line->p1() );
    }
    if( index < m_conLineList.size()-1 )
    {
        if( m_conLineList.size() < 2 ) return;

        connectLines( index, index+1 );
        m_conLineList.at( index+1 )->sSetP1( line->p2() );
    }
    line->setIsBus( m_isBus );
    if( Circuit::self()->is_constarted() ) line->setCursor( Qt::ArrowCursor );
}

ConnectorLine* Connector::addConLine( int x1, int y1, int x2, int y2, int index )
{
    ConnectorLine* line = new ConnectorLine( x1, y1, x2, y2, this );
    addConLine( line, index );
    return line;
}

void Connector::connectLines( int index1, int index2 )
{
    if( index1 < 0 || index2 < 0 || index2 > m_conLineList.length()-1 )
        return;

    ConnectorLine* line1 = m_conLineList.at( index1 );
    ConnectorLine* line2 = m_conLineList.at( index2 );

    line1->setNextLine( line2 );
    line2->setPrevLine( line1 );
}

void Connector::disconnectLines( int index1, int index2 )
{
    if( index1 < 0 || index2 < 0 || index2 > m_conLineList.length()-1 )
        return;

    ConnectorLine* line1 = m_conLineList.at( index1 );
    ConnectorLine* line2 = m_conLineList.at( index2 );

    line1->setNextLine( NULL );
    line2->setPrevLine( NULL );
}

void Connector::updateConRoute( Pin* pin, QPointF thisPoint )
{
    if( m_conLineList.isEmpty() ) return;
    if( !m_conLineList.first()->isVisible() ) return;
    if( Circuit::self()->pasting() )  { remNullLines(); return; }

    bool diagonal = false;
    int length = m_conLineList.length();
    ConnectorLine* line;
    ConnectorLine* preline = NULL;

    if( pin == m_startPin )
    {
        line = m_conLineList.first();
        diagonal = line->isDiagonal();
        line->sSetP1( thisPoint.toPoint() );
        m_lastindex = 0;

        if( length > 1 )
        {
            preline = m_conLineList.at(1);
            m_actLine = 1;
        }
        else m_actLine = 0;
        
        if( diagonal ) { remNullLines(); return; }
    }else{
        line = m_conLineList.last();
        diagonal = line->isDiagonal();
        
        line->sSetP2( togrid( thisPoint ).toPoint() );
        
        m_lastindex = length-1;

        if( length > 1 )
        {
            preline = m_conLineList.at( m_lastindex-1 );
            if( pin != NULL ) m_actLine = m_lastindex-1;
        }
        if( diagonal || m_freeLine ) 
        {
            m_freeLine = false;
            if( m_lastindex == m_actLine )          // Add new corner
            {
                QPoint point = line->p2();
                ConnectorLine* newLine = addConLine( point.x(), point.y(), point.x()+4, point.y()+4, m_lastindex + 1 );
                if( line->isSelected() ) newLine->setSelected( true );
            }
            remNullLines();
            return;
    }   }
    if( (line->dx() == 0) && (line->dy() == 0) && (length > 1) ) // Null Line
    {
        if( line->scene() ) Circuit::self()->removeItem( line );
        m_conLineList.removeOne( line );

        if( m_actLine > 0 )  m_actLine -= 1;
    }
    else if( line->dx() != 0 && line->dy() != 0 )
    {
        QPoint point;

        if( m_lastindex == m_actLine )          // Add new corner
        {
            point = line->p2();

            if( abs(line->dx()) > abs(line->dy()) ) point.setY( line->p1().y() );
            else                                    point.setX( line->p1().x() );

            ConnectorLine* newLine = addConLine( point.x(), point.y(), line->p2().x(), line->p2().y(), m_lastindex + 1 );

            if( line->isSelected() ) newLine->setSelected( true );
            line->setP2( point );
        }
        else if( m_lastindex < m_actLine )        // Update first corner
        {
            point = line->p2();

            if     ( preline->dx() == 0 ) point.setY( line->p1().y() );
            else /*if( preline->dy() == 0 )*/ point.setX( line->p1().x() );

            line->setP2( point );

            if( line->dx() == preline->dx() || line->dy() == preline->dy() ) // Lines aligned or null line
            {
                if( line->isSelected() || preline->isSelected())
                {
                    preline->sSetP1( line->p1() );
                    remConLine( line  );
        }   }   }
        else{                                      // Update last corner
            point = line->p1();

            if     ( preline->dx() == 0 ) point.setY( line->p2().y() );
            else /*if( preline->dy() == 0 )*/ point.setX( line->p2().x() );

            line->setP1( point );

            if( line->dx() == preline->dx() || line->dy() == preline->dy() ) // Lines aligned or null line
            {
                if( line->isSelected() || preline->isSelected())
                {
                    preline->sSetP2( line->p2() );
                    remConLine( line  );
    }   }   }   }
    remNullLines();
}

void Connector::remLines()
{
    while( !m_conLineList.isEmpty() )
    {
        ConnectorLine* line = m_conLineList.takeLast();
        if( line->scene() ) Circuit::self()->removeItem( line );
        delete line;
}   }

void Connector::move( QPointF delta )
{
    if( Circuit::self()->pasting() )
    {
        for( ConnectorLine* line : m_conLineList )
            line->move( delta );
}   }

void Connector::setSelected(  bool selected )
{
    for( ConnectorLine* line : m_conLineList ) line->setSelected( selected );
    //setSelected( selected );
}

void Connector::setVisib(  bool vis )
{
    for( ConnectorLine* line : m_conLineList ) line->setVisible( vis );
}

void Connector::remove()
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    if( m_startPin ) m_startPin->reset();
    if( m_endPin )   m_endPin->reset();

    Circuit::self()->conList()->removeOne( this );

    remLines();
}

void Connector::closeCon( Pin* endpin, bool connect  )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    m_endPin   = endpin;
    m_endpinid = endpin->objectName();

    if( connect ){
        QString enodid = "enode";
        enodid.append( m_id );
        enodid.remove( "Connector" );
        eNode* newEnode = new eNode( enodid );
        if( m_isBus ) newEnode->setIsBus( true );

        // We will get all ePins from stratPin and endPin nets an add to new eNode
        m_startPin->setConPin( NULL );
        m_endPin->setConPin( NULL );

        m_startPin->registerEnode( newEnode );
        m_endPin->registerEnode( newEnode );
    }
    m_startPin->setConnector( this );
    m_endPin->setConnector( this );
    
    if( m_isBus ){
        m_startPin->setIsBus( true );
        m_endPin->setIsBus( true );
    }
    m_startPin->setConPin( m_endPin );
    m_endPin->setConPin( m_startPin );
    updateConRoute( m_endPin, m_endPin->scenePos() );

    for( ConnectorLine* line : m_conLineList ) line->setCursor( Qt::CrossCursor );
}

void Connector::splitCon( int index, Pin* pin1, Pin* pin2 )
{
    if( !m_endPin ) return;

    pin2->setEnode( enode() );
    pin1->setEnode( enode() );

    disconnectLines( index-1, index );

    QString type = QString("Connector");
    QString id = type;
    id.append( "-" );
    id.append( Circuit::self()->newSceneId() );

    Connector* new_connector = new Connector( Circuit::self(), type, id, pin2 );
    Circuit::self()->conList()->append( new_connector );

    int newindex = 0;
    int size = m_conLineList.size();
    for( int i = index; i < size; ++i)
    {
        ConnectorLine* lline = m_conLineList.takeAt( index );
        new_connector->lineList()->insert( newindex, lline );

        lline->setParent( new_connector );
        lline->setConnector( new_connector );

        if( newindex > 1 ) new_connector->incActLine();
        ++newindex;
    }
    if( index > 1 )  m_actLine = index-2;
    else             m_actLine = 0;
    
    new_connector->closeCon( m_endPin );    // Close new_connector first please
    closeCon( pin1 );                       // Close this
}

void Connector::updateLines()
{
    eNode* enode = startPin()->getEnode();
    if( enode && enode->voltchanged() )
    {
        for( ConnectorLine*  line : m_conLineList ) line->update();
}   }

QString Connector::enodId() 
{
    eNode *node = m_startPin->getEnode();
    if( node ) return node->itemId();
    return "";
}

eNode* Connector::enode() { return m_startPin->getEnode(); }
void   Connector::setEnode( eNode* enode )
{ 
    if( m_startPin ) m_startPin->setEnode( enode );
    if( m_endPin )   m_endPin->setEnode( enode );
    if( m_isBus )    enode->setIsBus( true );
}

void Connector::setIsBus( bool bus )
{
    if( m_isBus == bus ) return;
    for( ConnectorLine* line : m_conLineList ) line->setIsBus( bus );
    m_isBus = bus;
}

double Connector::getVolt() { return m_startPin->getVolt(); }

//#include "moc_connector.cpp"
