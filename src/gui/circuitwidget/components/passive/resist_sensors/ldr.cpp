/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include<math.h>

#include "ldr.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "intprop.h"
#include "propdialog.h"

Component* Ldr::construct( QObject* parent, QString type, QString id )
{ return new Ldr( parent, type, id ); }

LibraryItem* Ldr::libraryItem()
{
    return new LibraryItem(
        tr( "LDR" ),
        "Resistive Sensors",
        "ldr.png",
        "LDR",
        Ldr::construct);
}

Ldr::Ldr( QObject* parent, QString type, QString id )
   : VarResBase( parent, type, id  )
{
    m_r1    = 127410;
    m_gamma = 0.8582;

    setVal( 1 );

    addPropGroup( { tr("Main"), {
new DoubProp<Ldr>( "Lux"      , tr("Current Value"),"Lux", this, &Ldr::getVal,  &Ldr::setVal ),
new DoubProp<Ldr>( "Min_Lux"  , tr("Minimum Value"),"Lux", this, &Ldr::minVal,  &Ldr::setMinVal ),
new DoubProp<Ldr>( "Max_Lux"  , tr("Maximum Value"),"Lux", this, &Ldr::maxVal,  &Ldr::setMaxVal ),
new DoubProp<Ldr>( "Dial_Step", tr("Dial Step")    ,"Lux", this, &Ldr::getStep, &Ldr::setStep )
    },0} );
    addPropGroup( { tr("Parameters"), {
new DoubProp<Ldr>( "Gamma", tr("Gamma"),"" , this, &Ldr::gamma, &Ldr::setGamma ),
new IntProp <Ldr>( "R1"   , tr("R1")   ,"Ω", this, &Ldr::r1,    &Ldr::setR1,0,"uint" )
    },0} );
    addPropGroup( { tr("Dial"), Dialed::dialProps(), 0} );
}
Ldr::~Ldr(){}

void Ldr::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    double res = double(m_r1)*pow( m_value, -m_gamma );
    eResistor::setRes( res );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Lux" ) setValLabelText( getPropStr( "Lux" ) );
}

void Ldr::setR1( int r1 )
{
    m_r1 = r1;
    m_needUpdate = true;
}

void Ldr::setGamma( double ga )
{
    m_gamma = ga;
    m_needUpdate = true;
}

void Ldr::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    p->drawRect( QRectF(-11,-4.5, 22, 9 ));
    
    p->drawLine(-5,-11,-1,-7 );
    p->drawLine(-1, -7,-1,-9 );
    p->drawLine(-1, -7,-3,-7 );

    p->drawLine( 1,-11, 5,-7 );
    p->drawLine( 5, -7, 5,-9 );
    p->drawLine( 5, -7, 3,-7 );
}
