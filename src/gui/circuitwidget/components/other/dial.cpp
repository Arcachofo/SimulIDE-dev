/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QAbstractSlider>
#include <QPainter>
#include <QMenu>
#include <QGraphicsProxyWidget>

#include "dial.h"
#include "dialwidget.h"
#include "propdialog.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"

#include "boolprop.h"
#include "stringprop.h"

Component* Dial::construct( QObject* parent, QString type, QString id )
{ return new Dial( parent, type, id ); }

LibraryItem* Dial::libraryItem()
{
    return new LibraryItem(
        tr( "Dial" ),
        "Other",
        "dial.png",
        "Dial",
        Dial::construct );
}

Dial::Dial( QObject* parent, QString type, QString id )
    : Dialed( parent, type, id )
{
    m_graphical = true;
    m_linkable  = true;
    m_area = QRectF(-12,-4.5, 24, 12.5 );

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);

    Dial::updateProxy();

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Dial"), Dialed::dialProps(), groupNoCopy } );
    addPropGroup( { "Hidden", {
new StrProp<Dial>("Links", "Links","", this, &Dial::getLinks , &Dial::setLinks )
    }, groupHidden} );
}
Dial::~Dial() {}

void Dial::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    int v = m_dialW.value();
    for( int i=0; i<m_linkedComp.size(); ++i )
    {
        Component* comp = m_linkedComp.at( i );
        comp->setLinkedValue( v );  //update();
    }
}

/*void Dial::compSelected( Component* comp )
{

    Linkable::compSelected( comp );
    if( !Simulator::self()->isRunning() ) updateStep();
}*/


void Dial::updateProxy()
{
    m_proxy->setPos( QPoint(-m_dialW.width()/2,-m_dialW.height()-5) );
}

void Dial::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) { event->ignore(); return; }

    event->accept();
    QMenu* menu = new QMenu();

    QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
    connect( linkCompAction, &QAction::triggered,
                       this, &Dial::slotLinkComp, Qt::UniqueConnection );

    menu->addSeparator();

    Component::contextMenu( event, menu );
    menu->deleteLater();
}

void Dial::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );
    p->drawRect( m_area );
}
