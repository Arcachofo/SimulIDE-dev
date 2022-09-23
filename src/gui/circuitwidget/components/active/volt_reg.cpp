/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>
#include <QPainter>

#include "volt_reg.h"
#include "connector.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "pin.h"
#include "e-node.h"

#include "doubleprop.h"

Component* VoltReg::construct( QObject* parent, QString type, QString id )
{ return new VoltReg( parent, type, id ); }

LibraryItem* VoltReg::libraryItem()
{
    return new LibraryItem(
        tr( "Volt. Regulator" ),
        tr( "Active" ),
        "voltreg.png",
        "VoltReg",
        VoltReg::construct );
}

VoltReg::VoltReg( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eResistor( id )
{
    m_area = QRect( -11, -8, 22, 19 );

    setValLabelPos( 15, 12, 0 );

    m_voltPos = 0;
    m_voltNeg = 0;
    
    m_pin.resize( 3 );
    m_ePin.resize( 3 );

    m_ePin[0] = m_pin[0] = new Pin( 180, QPoint( -16, 0 ), id+"-input", 0, this );
    m_pin[0]->setLength(6);
    m_pin[0]->setLabelText( "I" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );

    m_ePin[1] = m_pin[1] = new Pin( 0, QPoint( 16, 0 ), id+"-output", 1, this );
    m_pin[1]->setLength(6);
    m_pin[1]->setLabelText( "O" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );

    m_ePin[2] = m_pin[2] = new Pin( 270, QPoint( 0, 16 ), id+"-ref", 2, this );
    m_pin[2]->setLength(6);
    m_pin[2]->setLabelText( "R" );
    m_pin[2]->setLabelColor( QColor( 0, 0, 0 ) );

    addPropGroup( { tr("Main"), {
new DoubProp<VoltReg>( "Voltage", tr("Output Voltage"),"V", this, &VoltReg::outVolt, &VoltReg::setOutVolt )
    }} );

    setShowProp("Voltage");
    setPropStr( "Voltage", "1.2" );
}
VoltReg::~VoltReg(){}

void VoltReg::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToNoLinList(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToNoLinList(this);
    if( m_ePin[2]->isConnected() ) m_ePin[2]->getEnode()->addToNoLinList(this);

    m_ePin[0]->createCurrent();
    m_ePin[1]->createCurrent();

    eResistor::stamp();
    eResistor::setRes( 1e-6 );
    m_accuracy = Simulator::self()->NLaccuracy();
    m_lastOut = 0;
}

void VoltReg::voltChanged()
{
    double inVolt = m_pin[0]->getVoltage();

    m_voltPos = inVolt;
    if( m_voltPos > 0.7 ) m_voltPos -= 0.7;
    else                  m_voltPos = 0;

    double outVolt = m_ePin[2]->getVoltage()+m_vRef;

    if     ( outVolt > m_voltPos ) outVolt = m_voltPos;
    else if( outVolt < m_voltNeg ) outVolt = m_voltNeg;

    double current = (inVolt-outVolt)*m_admit;

    if( qFabs(current-m_lastOut)<m_accuracy ) return;
    m_lastOut = current;

    Simulator::self()->notCorverged();

    m_pin[0]->stampCurrent( current );
    m_pin[1]->stampCurrent(-current );
}


void VoltReg::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );
}
