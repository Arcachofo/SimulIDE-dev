/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include "e-function.h"
#include "simulator.h"

eFunction::eFunction( QString id )
         : eLogicDevice( id )
         , m_engine()
         , m_functions()
{
}
eFunction::~eFunction()
{
}

void eFunction::stamp()
{
    eLogicDevice::stamp();
    
    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_program.clear();
    for( int i=0; i<m_numOutputs; ++i )
    {
        m_program.append( QScriptProgram( m_funcList.at(i) ));
    }
}

void eFunction::voltChanged()
{
    //uint bits = 0;
    uint bit = 0;
    //uint msb = (m_numInputs+m_numOutputs)*2-1;
    for( int i=0; i<m_numInputs; ++i )
    {
        bit = eLogicDevice::getInputState( i );
        //if( bit ) bits += 1 << (msb-(i*4));
        //else      bits += 1 << (msb-(i*4)-1);
        m_engine.globalObject().setProperty( "i"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vi"+QString::number(i), QScriptValue( m_input[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "inBits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "inputs", QScriptValue( m_numInputs ) );

    for( int i=0; i<m_numOutputs; ++i )
    {
        bit = eLogicDevice::getOutputState( i );
        //if( bit ) bits += 1 << (msb-(i*4)-2);
        //else      bits += 1 << (msb-(i*4)-3);
        m_engine.globalObject().setProperty( "o"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vo"+QString::number(i), QScriptValue( m_output[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "bits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "outputs", QScriptValue( m_numOutputs ) );
    Simulator::self()->addEvent( m_propDelay, this );
}

void eFunction::runEvent()
{
    for( int i=0; i<m_numOutputs; ++i )
    {
        if( i >= m_numOutputs ) break;
        QString text = m_funcList.at(i).toLower();
        
        //qDebug() << "eFunction::voltChanged()"<<text<<m_engine.evaluate( text ).toString();
            
        if( text.startsWith( "vo" ) )
        {
            float out = m_engine.evaluate( m_program.at(i) ).toNumber();
            m_output[i]->setVoltHigh( out );
            m_output[i]->setOut( true );
            m_output[i]->stampOutput();
            
        }
        else
        {
            bool out = m_engine.evaluate( m_program.at(i) ).toBool();
            m_output[i]->setTimedOut( out );
        }
        //qDebug()<<"Func:"<< i << text; //textLabel->setText(text);
        //qDebug() << ":" << out;
        //qDebug() << m_engine.globalObject().property("i0").toVariant() << m_engine.globalObject().property("i1").toVariant();
    }
}

QString eFunction::functions()
{
    return m_functions;
}

void eFunction::setFunctions( QString f )
{
    //qDebug()<<"eFunction::setFunctions"<<f;
    if( f.isEmpty() ) return;
    m_functions = f;
    m_funcList = f.split(",");
}
