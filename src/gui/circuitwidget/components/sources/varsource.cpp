/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QDial>
#include <QPainter>

#include "varsource.h"
#include "simulator.h"
#include "circuit.h"

#define WIDTH 40
#define HEIGHT 56
#define DIAL_SIZE 36

VarSource::VarSource( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , eElement( id )
{
    m_area = QRect( -WIDTH/2, -HEIGHT/2, WIDTH, HEIGHT );

    m_graphical = true;
    m_changed = false;
    m_outValue = 5.0;

    m_voltw.setFixedSize( WIDTH-4, HEIGHT-4 );
    
    m_proxy = Circuit::self()->addWidget( &m_voltw );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2+2, -HEIGHT/2+2 ) );
    //m_proxy->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true );

    m_button = m_voltw.pushButton;
    m_button->setFixedSize( WIDTH-8,16 );

    m_dial = m_voltw.dial;
    m_dial->setMaximum( 1000 );

    setValLabelPos( -8, 30 , 0 ); // x, y, rot
    setLabelPos(-32,-48, 0);

    Simulator::self()->addToUpdateList( this );

    connect( m_button, SIGNAL( clicked()),
             this,     SLOT  ( onbuttonclicked()), Qt::UniqueConnection );

    connect( m_dial,   SIGNAL( valueChanged(int) ),
             this,     SLOT  ( valueChanged(int)), Qt::UniqueConnection );
}
VarSource::~VarSource() { }

void VarSource::initialize(){ m_changed = true; }

void VarSource::updateButton()
{
    QString msg;
    bool checked = m_button->isChecked();
    
    if( checked )
          msg = QString("%1 "+m_unit).arg(float(int(m_outValue*100+0.5))/100);
    else  msg = QString("--"+m_unit);
        
    m_button->setText( msg );
}

void VarSource::onbuttonclicked()
{
    updateButton();
    m_changed = true;
}

void VarSource::valueChanged( int val )
{
    m_outValue = double( m_maxValue*val/1000 );
    updateButton();
    m_changed = true;
}

void VarSource::setVal( double val )
{
    if( val > m_maxValue ) val = m_maxValue;
    else if( val < 0 ) val = 0;
    m_dial->setValue( val*1000/m_maxValue );
    m_outValue = val;
    m_changed = true;
    updateButton();
    if( !Simulator::self()->isRunning() ) updateStep();
}

void VarSource::setflip()
{
    Component::setflip();
    m_proxy->setPos( QPoint( (-WIDTH/2+2)*m_Hflip, (-HEIGHT/2+2)*m_Vflip ) );
    m_proxy->setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
}

void VarSource::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 2, 2 );

    //p->fillRect( m_area, Qt::darkGray );
}

#include "moc_varsource.cpp"
