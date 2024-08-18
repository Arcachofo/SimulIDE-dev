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

    m_width  = 24;
    m_height = 12;
    m_border = 2;
    updateArea();

    m_graphical = true;
    m_isLinker = true;

    m_minVal = 0;
    m_maxVal = 999;
    m_steps  = 1000;

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);

    Dial::updateProxy();

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"),
    {
        new IntProp<Dial>("Min_Val", tr("Minimum Value"), ""
                         , this, &Dial::minVal, &Dial::setMinVal ),

        new IntProp<Dial>("Max_Val", tr("Maximum Value"), ""
                         , this, &Dial::maxVal, &Dial::setMaxVal ),

        new IntProp<Dial>("Steps", tr("Steps"), ""
                         , this, &Dial::steps, &Dial::setSteps )
    },0 } );

    addPropGroup( { tr("Dial"), Dialed::dialProps(), groupNoCopy } );


    addPropGroup( { tr("Body"), {
        new IntProp <Dial>("width", tr("Width"), "_px"
                           , this, &Dial::width, &Dial::setWidth,0,"uint" ),

        new IntProp <Dial>("height", tr("Height"), "_px"
                           , this, &Dial::height, &Dial::setHeight,0,"uint" ),

        new IntProp <Dial>("Border", tr("Border"), "_px"
                           , this, &Dial::border, &Dial::setBorder ),

        new StrProp <Dial>("Background", tr("Background"),""
                           , this, &Dial::background, &Dial::setBackground ),
    },0} );

    addPropGroup( { "Hidden",
    {
        new StrProp<Dial>("Links", "Links",""
                         , this, &Dial::getLinks , &Dial::setLinks ),

        new IntProp<Dial>("Value", "", ""
                         , this, &Dial::value, &Dial::setValue ),
    }, groupHidden } );
}
Dial::~Dial() {}

void Dial::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    double v = m_dialW.value();
    double range = m_maxVal - m_minVal;
    v = m_minVal + v*range/(m_steps-1);
    for( int i=0; i<m_linkedComp.size(); ++i )
    {
        Component* comp = m_linkedComp.at( i );
        comp->setLinkedValue( v );  //update();
    }
}

int Dial::value()
{
    return m_dialW.dial()->value();
}

void Dial::setValue( int v )
{
    m_dialW.dial()->setValue( v );
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

void Dial::setSteps( int s )
{
    if( s < 2 ) s = 2;
    if( m_steps == s ) return;
    m_steps = s;
    m_dialW.setMaximum( s-1 );

    int single = s/40;
    if( single < 1 ) single = 1;
    m_dialW.setSingleStep( single );
}

void Dial::setWidth( int width )
{
    if( width < 1 ) width = 1;
    if( m_width == width ) return;

    m_width = width;
    updateArea();
    Circuit::self()->update();
}

void Dial::setHeight( int height )
{
    if( height < 1 ) height = 1;
    if( m_height == height ) return;

    m_height = height;
    updateArea();
    Circuit::self()->update();
}

void Dial::updateArea()
{
    m_area = m_areaComp = QRectF(-double(m_width)/2,-4, m_width, m_height);
    update();
}

void Dial::setBorder( int border )
{
    if( border < 0 ) border = 0;
    m_border = border;
    update();
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

    if( !parentItem() )
    {
        QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
        QObject::connect( linkCompAction, &QAction::triggered, [=](){ slotLinkComp(); } );

        menu->addSeparator();
    }
    Component::contextMenu( event, menu );
    menu->deleteLater();
}

void Dial::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    if( m_hidden ) return;

    Component::paint( p, o, w );

    if( m_backPixmap ) p->drawPixmap( QRect(m_area.x(), m_area.y(), m_width, m_height), *m_backPixmap );
    else{
        QPen pen( Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p->setPen( pen );
        p->drawRect( m_area );
    }
    Component::paintSelected( p );
}
