/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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
// Based on Falstad Circuit Simulator Diode model: https://falstad.com

#include <math.h>
#include <QDir>
#include <QDomDocument>

#include "e-diode.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"
#include "mainwindow.h"
#include "utils.h"

QHash<QString, diodeData_t> eDiode::m_diodes;
QHash<QString, diodeData_t> eDiode::m_zeners;
QHash<QString, diodeData_t> eDiode::m_leds;

eDiode::eDiode( QString id )
      : eResistor( id )
{
    m_vt = 0.025865;
    m_vzCoef = 1/m_vt;
    m_maxCur = 1;
}
eDiode::~eDiode(){}

void eDiode::initialize()
{
    m_admit = m_bAdmit;
    m_voltPN = 0;
    m_current = 0;
}

void eDiode::stamp()
{
    eNode* node = m_ePin[0]->getEnode();
    if( node ) node->addToNoLinList( this );

    node = m_ePin[1]->getEnode();
    if( node ) node->addToNoLinList( this );

    eResistor::stamp();
}

void eDiode::voltChanged()
{
    double voltPN = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();
    if( fabs( voltPN - m_voltPN ) < .01 ) { m_step = 0; return; } // Converged  /// Mingw needs fabs
    Simulator::self()->notCorverged();

    m_step += .01;
    double gmin = m_bAdmit*exp( m_step );
    if( gmin > .1 ) gmin = .1;

    if( m_bkDown == 0 || voltPN >= 0 )  // No breakdown Diode or Forward biased Zener
    {
        voltPN = limitStep( voltPN, m_vScale, m_vCriti );
        double eval = exp( voltPN*m_vdCoef );
        m_admit = m_satCur*m_vdCoef*eval + gmin;
        m_current = m_satCur*(eval-1);
    }
    else{                               // Reverse biased Zener or Diode with breakdown
        double volt = -voltPN-m_zOfset;
        voltPN = -( limitStep( volt, m_vt, m_vzCrit ) + m_zOfset );
        double eval = exp( voltPN*m_vdCoef );
        double expCoef = exp( volt*m_vzCoef );
        m_admit = m_satCur*( m_vdCoef*eval - m_vzCoef*m_vzCoef*expCoef ) + gmin;
        m_current = m_satCur*( eval - expCoef - 1 );
    }
    m_voltPN = voltPN;

    eResistor::stampAdmit();

    double stCurr = m_current - m_admit*voltPN;
    m_ePin[0]->stampCurrent(-stCurr );
    m_ePin[1]->stampCurrent( stCurr );
}

inline double eDiode::limitStep( double vnew, double scale, double vc )
{
    if( vnew > vc && abs( vnew - m_voltPN ) > scale*2 ) // check new voltage; has current changed by factor of e^2?
    {
        if( m_voltPN > 0 )
        {
            double arg = 1 + (vnew - m_voltPN)/scale;
            if( arg > 0 ) vnew = m_voltPN + scale*log( arg );
            else          vnew = vc;
        }else             vnew = scale*log( vnew/scale );
    } return vnew;
}

void eDiode::SetParameters( double sc, double ec, double bv, double sr )
{
    m_satCur = sc;
    m_emCoef = ec;
    m_bkDown = bv;
    setResSafe( sr );

    updateValues();
}

void eDiode::setThreshold( double fdDrop )
{
    m_satCur = 1/( exp( fdDrop*m_vdCoef ) - 1 );
    updateValues();
}

void eDiode::setSatCur( double satCur )
{
    m_satCur = satCur;
    updateValues();
}

void eDiode::setEmCoef( double emCoef )
{
    m_emCoef = emCoef;
    updateValues();
}

void  eDiode::setBrkDownV( double bkDown )
{
    if( bkDown >= 0 ) m_bkDown = bkDown;
    else              m_bkDown = -bkDown;
    updateValues();
}

void eDiode::updateValues()
{
    m_vScale = m_emCoef*m_vt;
    m_vdCoef = 1/m_vScale;
    m_fdDrop = m_vScale*log( 1/m_satCur + 1 );
    m_vCriti = m_vScale*log( m_vScale/(sqrt(2)*m_satCur) );
    m_zOfset = m_bkDown - m_vt*log(-(1-0.005/m_satCur) );
    m_vzCrit = m_vt*log( m_vt/(sqrt(2)*m_satCur) );
    m_bAdmit = m_satCur*1e-2;
}

void eDiode::getModels() // Static
{
    m_diodes.insert( "Custom", {0, 0, 0, 0} );
    m_diodes.insert( "Diode Default", {171.4352819281, 2, 0, 0.05} );
    m_zeners.insert( "Custom", {0, 0, 0, 0} );
    m_zeners.insert( "Zener Default", {171.4352819281, 2, 5.6, 0.05} );
    m_leds.insert( "Custom", {0, 0, 0, 0} );
    m_leds.insert( "RGY Default", {0.0932, 3.73, 0, 0.042} );

    QString modelsFile = MainWindow::self()->getFilePath( "data/diodes.model" );

    QDomDocument domDoc = fileToDomDoc( modelsFile, "Diode::getModels");
    QDomNode node = domDoc.documentElement().firstChild();
    while( !node.isNull() )
    {
        QDomElement itemset = node.toElement();
        const QString type = itemset.attribute("type").toLower();

        if( itemset.tagName() == "itemset" )
        {
            QDomNode nodei = itemset.firstChild();
            while( !nodei.isNull() )
            {
                QDomElement item = nodei.toElement();
                if( item.tagName() == "item" )
                {
                    QString name = item.attribute("name");
                    diodeData_t data;
                    data.satCur = item.attribute("satCurr_nA").toDouble();
                    data.emCoef = item.attribute("emCoef").toDouble();
                    data.brkDow = item.attribute("brkDown").toDouble();
                    data.resist = item.attribute("resist").toDouble();

                    if     ( type == "diode") m_diodes.insert( name, data );
                    else if( type == "zener") m_zeners.insert( name, data );
                    else if( type == "led")   m_leds.insert( name, data );
                }
                nodei = nodei.nextSibling();
        }   }
        node = node.nextSibling();
}   }

void eDiode::setModel( QString model )
{
    m_model = model;
    if( model == "Custom" ) return;
    if     ( m_diodes.contains( model ) ) setModelData( m_diodes.value( model ) );
    else if( m_zeners.contains( model ) ) setModelData( m_zeners.value( model ) );
    else if( m_leds.contains( model ) )   setModelData( m_leds.value( model ) );
}

void eDiode::setModelData( diodeData_t data )
{
    SetParameters( data.satCur*1e-9, data.emCoef, data.brkDow, data.resist );
}

