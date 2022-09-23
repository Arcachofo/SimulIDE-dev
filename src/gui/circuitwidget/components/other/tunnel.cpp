/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "tunnel.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "e-node.h"
#include "pin.h"
#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"

QHash<QString, eNode*> Tunnel::m_eNodes;
QHash<QString, QList<Tunnel*>*> Tunnel::m_tunnels;

Component* Tunnel::construct( QObject* parent, QString type, QString id )
{ return new Tunnel( parent, type, id ); }

LibraryItem* Tunnel::libraryItem()
{
    return new LibraryItem(
        tr( "Tunnel." ),
        tr( "Connectors" ),
        "tunnel.png",
        "Tunnel",
        Tunnel::construct );
}

Tunnel::Tunnel( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
{
    m_size = 20;
    m_area = QRect( -m_size-8-4, -4, m_size+4, 8 );
    m_rotated = false;
    m_blocked = false;
    m_packed  = false;
    m_name = "";

    m_pin.resize( 1 );
    m_pin[0] = new Pin( 0, QPoint(0,0), id+"-pin", 0, this);
    m_pin[0]->setLabelColor( Qt::black );

    setLabelPos(-16,-24, 0);

    addPropGroup( { tr("Main"), {
new StringProp<Tunnel>( "Name"   , tr("Id")    ,"", this, &Tunnel::name,    &Tunnel::setName ),
//new BoolProp  <Tunnel>( "Rotated",tr("Rotated"),"", this, &Tunnel::rotated, &Tunnel::setRotated )
    }} );
    addPropGroup( {"Hidden", {
new StringProp<Tunnel>( "Uid","","", this, &Tunnel::tunnelUid, &Tunnel::setTunnelUid ),
    }} );
}
Tunnel::~Tunnel() {}

bool Tunnel::setPropStr( QString prop, QString val )
{
    if( prop =="Rotated" )       // Old: TODELETE
    {
        if( val == "true" ) { m_Hflip = -1; setflip(); }
    }
    else return Component::setPropStr( prop, val );
    return true;
}

void Tunnel::setEnode( eNode* node )
{
    if( m_blocked ) return;
    m_blocked = true;

    if( node ) m_pin[0]->registerPinsW( node );
    m_blocked = false;
}

void Tunnel::registerEnode( eNode* enode, int ) // called by m_pin[0]
{
    if( m_blocked ) return;

    QList<Tunnel*>* list = m_tunnels.value( m_name );
    if( !list ) return;

    m_eNodes[m_name] = enode;
    m_blocked = true;

    for( Tunnel* tunnel: *list )
        if( tunnel != this ) tunnel->setEnode( enode );

    m_blocked = false;
}

void Tunnel::setName( QString name )
{
    if( name == m_name ) return;

    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    removeTunnel(); // Remove old name before setting new one

    m_name = name;
    if( !m_packed ) m_pin[0]->setLabelText( name );
    if( name == "" ) m_size = 20;
    else  m_size = snapToGrid( m_pin[0]->labelSizeX()+4 );
    setRotated( m_rotated );

    if( name.isEmpty() ) { setEnode( NULL ); return; }

    if( m_tunnels.contains( name ) ) // There is already tunnel with this name
    {
        QList<Tunnel*>* list = m_tunnels.value( name );
        if( !list->contains( this ) ) list->append( this );
    }
    else   // name doesn't exist: Create a new List for this name
    {
        QList<Tunnel*>* list = new QList<Tunnel*>();
        list->append( this );
        m_tunnels[name] = list;
    }
    Circuit::self()->update();
}

void Tunnel::setRotated( bool rot )
{
    m_rotated  = rot;
    if( rot ){
        m_area = QRect( 4, -4, m_size+4, 8 );
        m_pin[0]->setPinAngle( 180 );
    }else {
        m_area = QRect( -m_size-8, -4, m_size+4, 8 );
        m_pin[0]->setPinAngle( 0 );
    }
    m_pin[0]->setLabelPos();
    Circuit::self()->update();
}

void Tunnel::removeTunnel()
{
    if( m_name.isEmpty() ) return;

    QList<Tunnel*>* list = m_tunnels.value( m_name );
    if( list )
    {
        list->removeAll( this );
        if( !list->isEmpty() ) return;

        m_tunnels.remove( m_name );
        m_eNodes.remove( m_name );
        delete list;
}   }

void Tunnel::remove()
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    removeTunnel();
    Component::remove();
    Circuit::self()->update();
}

QRectF Tunnel::boundingRect() const
{
    if( m_packed ) return QRectF( 0, 0, 0 ,0 );
    else return Component::boundingRect();
}

void Tunnel::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( !m_selMainCo )  // Used when creating Boards to set this as main component
        Component::mousePressEvent( event );
}

void Tunnel::paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    if( m_hidden || m_packed ) return;

    if( m_tunnels.contains( m_name ) ) m_color = QColor( 255, 255, 250 );
    else                               m_color = QColor( 210, 210, 230 );

    Component::paint( p, option, widget );

    if( m_rotated ){
        QPointF points[5] =        {
            QPointF( m_size+8,-4 ),
            QPointF(  8,-4 ),
            QPointF(  4, 0 ),
            QPointF(  8, 4 ),
            QPointF( m_size+8, 4 ) };

        p->drawPolygon( points, 5 );
    } else {
        QPointF points[5] =        {
            QPointF(-m_size-8,-4 ),
            QPointF(  -8,-4 ),
            QPointF(  -4, 0 ),
            QPointF(  -8, 4 ),
            QPointF(-m_size-8, 4 ) };

        p->drawPolygon( points, 5 );
}   }
