/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
// Based on Falstad Circuit Simulator Diode model: https://falstad.com

#include <QtMath>
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

void eDiode::stamp()
{
    m_admit = m_bAdmit;
    m_voltPN = 0;
    m_current = 0;

    eResistor::stamp();

    eNode* node = m_ePin[0]->getEnode();
    if( node ) node->addToNoLinList( this );

    node = m_ePin[1]->getEnode();
    if( node ) node->addToNoLinList( this );

    m_ePin[0]->createCurrent();
    m_ePin[1]->createCurrent();
}

void eDiode::voltChanged()
{
    double voltPN = m_ePin[0]->getVoltage() - m_ePin[1]->getVoltage();

    if( m_changed ) m_changed = false;
    else if( qFabs( voltPN - m_voltPN ) < .01 ) { m_step = 0; return; } // Converged
    Simulator::self()->notCorverged();

    m_step += .01;
    double gmin = m_bAdmit*qExp( m_step );
    if( gmin > .1 ) gmin = .1;

    if( voltPN > m_vCriti && qFabs(voltPN - m_voltPN) > m_vScale*2 ) // check new voltage; has current changed by factor of e^2?
    {
        voltPN = limitStep( voltPN, m_voltPN, m_vScale, m_vCriti );
    }
    else if( m_bkDown != 0 && voltPN < 0 )
    {
        voltPN = -voltPN - m_zOfset;
        double vold = -m_voltPN - m_zOfset;

        if( voltPN > m_vzCrit && qFabs(voltPN - vold) > m_vt*2 )
            voltPN = limitStep( voltPN, vold, m_vt, m_vzCrit );
        voltPN = -(voltPN+m_zOfset);
    }
    m_voltPN = voltPN;

    double eval = qExp( voltPN*m_vdCoef );

    if( m_bkDown == 0 || voltPN >= 0  )  // No breakdown Diode or Forward biased Zener
    {
        m_admit   = m_satCur * m_vdCoef*eval + gmin;
        m_current = m_satCur * (eval-1);
    }else{                               // Reverse biased Zener or Diode with breakdown
        double expCoef = qExp( (-voltPN-m_zOfset)*m_vzCoef );
        m_admit   = m_satCur * ( m_vdCoef*eval + m_vzCoef*expCoef ) + gmin;
        m_current = m_satCur * ( eval-1 - expCoef ) ;
    }
    eResistor::stampAdmit();

    double stCurr = m_current - m_admit*voltPN;
    m_ePin[0]->stampCurrent(-stCurr );
    m_ePin[1]->stampCurrent( stCurr );
}

inline double eDiode::limitStep( double vnew, double vold, double scale, double vc )
{
    if( vold > 0 )
    {
        double arg = 1 + (vnew-vold)/scale;
        if( arg > 0 ) vnew = vold + scale*qLn( arg );
        else          vnew = vc;
    }else             vnew = scale*qLn( vnew/scale );

    return vnew;
}

void eDiode::SetParameters( double sc, double ec, double bv, double sr )
{
    m_satCur = sc;
    m_emCoef = ec;
    m_bkDown = bv;
    setResSafe( sr );

    updateValues();
}

void eDiode::setThreshold( double vCrit )
{
    if( vCrit < 0.01 ) return;
    m_satCur = m_vScale/(qExp(vCrit/m_vScale)*qSqrt(2));
    updateValues();
}

void eDiode::setSatCur( double satCur )
{
    if( satCur <= 0 ) return;
    m_satCur = satCur;
    updateValues();
}

void eDiode::setEmCoef( double emCoef )
{
    if( emCoef < 0.01 ) return;
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
    m_vCriti = m_vScale*qLn( m_vScale/(qSqrt(2)*m_satCur) );
    m_zOfset = m_bkDown - m_vt*qLn(-(1-0.005/m_satCur) );
    m_vzCrit = m_vt*qLn( m_vt/(qSqrt(2)*m_satCur) );
    m_bAdmit = m_satCur*1e-2;
    m_changed = true;
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

