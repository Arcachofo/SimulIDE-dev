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
#include "intprop.h"
#include "stringprop.h"

#define tr(str) simulideTr("Dial",str)

Component* Dial::construct( QString type, QString id )
{ return new Dial( type, id ); }

LibraryItem* Dial::libraryItem()
{
    return new LibraryItem(
        tr("Dial"),
        "Other",
        "dial.png",
        "Dial",
        Dial::construct );
}

Dial::Dial( QString type, QString id )
    : Dialed( type, id )
{
    m_areaDial = QRectF(-11,-28 , 22, 22 );
    m_areaComp = QRectF(-12,-4.5, 24, 12.5 );
    m_area     = m_areaComp;

    m_graphical = true;
    m_linkable  = true;

    m_minVal = 0;
    m_maxVal = 1000;

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);

    Dial::updateProxy();

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new IntProp<Dial>( "Min_Val", tr("Minimum Value"), "", this, &Dial::minVal, &Dial::setMinVal ),
new IntProp<Dial>( "Max_Val", tr("Maximum Value"), "", this, &Dial::maxVal, &Dial::setMaxVal )
    },0 } );
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
    int range = m_maxVal - m_minVal;
    v = m_minVal + v*range/1000;
    for( int i=0; i<m_linkedComp.size(); ++i )
    {
        Component* comp = m_linkedComp.at( i );
        comp->setLinkedValue( v );  //update();
    }
}

void Dial::setMinVal( int min )
{
    if( min < 0    ) min = 0;
    if( min > m_maxVal ) min = m_maxVal;
    m_minVal = min;

    m_needUpdate = true;
}

void Dial::setMaxVal( int max )
{
    if( max < 0 ) max = 0;
    if( max < m_minVal ) max = m_minVal;
    m_maxVal = max;

    m_needUpdate = true;
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
    QObject::connect( linkCompAction, &QAction::triggered, [=](){ slotLinkComp(); } );

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
