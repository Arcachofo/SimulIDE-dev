/***************************************************************************
 *   Copyright (C) 2020 by Benoit ZERR                                     *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include <QPainter>
#include <QtMath>

#include "rtd.h"
#include "simulator.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "propdialog.h"

Component* RTD::construct( QObject* parent, QString type, QString id )
{ return new RTD( parent, type, id ); }

LibraryItem* RTD::libraryItem()
{
    return new LibraryItem(
        tr( "RTD" ),
        "Resistive Sensors",
        "rtd.png",
        "RTD",
        RTD::construct );
}

RTD::RTD( QObject* parent, QString type, QString id )
   : ThermistorBase( parent, type, id )
{
    addPropGroup( { tr("Parameters"), {
//new IntProp<RTD>( "B" , "B" ,""  , this, &RTD::bVal, &RTD::setBval, "uint" ),
new DoubProp<RTD>( "R0", "R0","Ω" , this, &RTD::getR0,   &RTD::setR0 )
    }, 0} );
    addPropGroup( { tr("Dial"), Dialed::dialProps(), groupNoCopy } );
}
RTD::~RTD() {}

void RTD::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    //m_t0_tau = (double) (Simulator::self()->step())/1e6;

    //uint64_t time = Simulator::self()->circTime();

    //double dt = (double)time/1e6 - m_t0_tau;
    //double dr = sensorFunction( m_temp )-m_resist;

    //double res = m_resist + dr*(1.0 -exp(-dt/m_tau));
    double res = sensorFunction( m_value );
    eResistor::setRes( res );
   if( m_propDialog ) m_propDialog->updtValues();
   else if( m_showProperty == "Temp" ) setValLabelText( getPropStr( "Temp" ) );
    //m_lastTime = time;
}

double RTD::sensorFunction( double temp )
{
  // linear approximation
  // double r_sense = r0*(1.0+coef_temp*sense); // linear approximation
  // polynomial approximation (more realistic)
  double r_sense = m_r0*(1.0+coef_temp_a*temp+coef_temp_b*temp*temp);
  if (temp < 0) r_sense += m_r0*coef_temp_c*(temp-100)*qPow(temp,3.0);

  return r_sense;
}

void RTD::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    QFont font = p->font();
    font.setPixelSize(8);
    p->setFont( font );

    p->drawRect( QRectF(-11,-4.5, 22, 9 ));

    p->drawLine(-6, 6, 8,-8 );
    p->drawText(-8,-6, "+tº");
}
