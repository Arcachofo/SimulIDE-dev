/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <qtconcurrentrun.h>
#include <QHash>
#include <math.h>

#include "simulator.h"
#include "circuit.h"
#include "matrixsolver.h"
#include "updatable.h"
#include "outpaneltext.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "circmatrix.h"
#include "e-element.h"

Simulator* Simulator::m_pSelf = NULL;

Simulator::Simulator( QObject* parent )
         : QObject( parent )
{
    m_pSelf = this;

    m_matrix = new CircMatrix();

    m_fps = 20;
    m_timerId   = 0;
    m_timerTick = 50;
    m_stepsPS   = 1e6;
    m_stepSize  = 1e6;
    m_noLinAcc  = 5; // Non-Linear accuracy
    m_maxNlstp  = 100000;
    m_slopeSteps = 0;

    m_errors[0] = "";
    m_errors[1] = "Could not solve Matrix";
    m_errors[2] = "Add Event: NULL free event";
    m_errors[3] = "LAST_SIM_EVENT reached";

    m_warnings[1] = "NonLinear Not Converging";
    m_warnings[100] = "AVR crashed !!!";

    ///m_running = false;
    resetSim();

    m_RefTimer.start();
}
Simulator::~Simulator()
{
    m_CircuitFuture.waitForFinished();
    delete m_matrix;
}

inline void Simulator::solveMatrix()
{
    while( m_changedNode )
    {
        m_changedNode->stampMatrix();
        m_changedNode = m_changedNode->nextCH;
    }
    if( !m_matrix->solveMatrix() ) // Try to solve matrix, if not stop simulation
    {
        qDebug() << "ERROR: Simulator::solveMatrix(), Failed to solve Matrix";
        m_state = SIM_ERROR;
        m_error = 1;
}   }                                // m_matrix sets the eNode voltages

void Simulator::timerEvent( QTimerEvent* e )  //update at m_timerTick rate (50 ms, 20 Hz max)
{
    e->accept();

    if( m_state == SIM_WAITING ) return;

    if( m_error )
    {
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setError( m_errors.value( m_error ) );
        return;
    }
    if( m_warning > 0 )
    {
        int type = (m_warning < 100)? 1:2;
        CircuitWidget::self()->setMsg( m_warnings.value( m_warning), type );
        m_warning = -10;
    }
    else if( m_warning < 0 )
    { if( ++m_warning == 0 ) CircuitWidget::self()->setMsg( " Running ", 0 ); }

    if( !m_CircuitFuture.isFinished() ) // Stop remaining parallel thread
    {
        simState_t state = m_state;
        m_state = SIM_WAITING;
        m_CircuitFuture.waitForFinished();
        m_state = state;
    }

    if( Circuit::self()->animate() )
    {
        Circuit::self()->updateConnectors();
        for( eNode* enode : m_eNodeList ) enode->setVoltChanged( false );
    }
    for( Updatable* el : m_updateList ) el->updateStep();

    // Calculate Load
    uint64_t loop = 0;
    if( m_loopTime > m_refTime ) loop = m_loopTime-m_refTime;
    m_load = (m_load+100*loop/((double)m_timerTick*1e6))/2;

    // Get Real Simulation Speed
    m_realStepsPF = m_circTime-m_tStep;
    m_tStep   = m_circTime;
    m_refTime = m_RefTimer.nsecsElapsed();

    if( m_state == SIM_RUNNING ) // Run Circuit in a parallel thread
        m_CircuitFuture = QtConcurrent::run( this, &Simulator::runCircuit );

    uint64_t deltaRefTime = m_refTime-m_lastRefT;
    if( deltaRefTime >= 1e9 )               // We want steps per 1 Sec = 1e9 ns
    {
        m_realSpeed = (m_tStep-m_lastStep)*10.0/deltaRefTime;
        CircuitWidget::self()->setRate( m_realSpeed, m_load );
        m_lastStep = m_tStep;
        m_lastRefT = m_refTime;
    }
    CircuitView::self()->setCircTime( m_tStep );
}

void Simulator::runCircuit()
{
    solveCircuit(); // Solving matrix here save events in updateStep()

    eElement* event = m_firstEvent;
    uint64_t endRun = m_circTime + m_stepsPF*m_stepSize; // Run upto next Timer event
    uint64_t nextTime;

    while( event ){                             // Simulator event loop
        if( event->eventTime > endRun ) break;  // All events for this Timer Tick are done

        nextTime = m_circTime;
        while( m_circTime == nextTime )         // Run all event with same timeStamp
        {
            m_circTime = event->eventTime;
            m_firstEvent = event->nextEvent;    // free Event
            event->nextEvent = NULL;
            event->runEvent();                  // Run event callback
            event = m_firstEvent;
            if( event ) nextTime = event->eventTime;
            else break;
        }
        solveCircuit();
        if( m_state < SIM_RUNNING ) break;
        event = m_firstEvent;
    }
    if( m_state > SIM_WAITING ) m_circTime = endRun;
    m_loopTime = m_RefTimer.nsecsElapsed();
}

void Simulator::solveCircuit()
{
    while( m_changedNode || m_nonLin )
    {
        if( m_changedNode ){
            solveMatrix();
            if( m_error ) return;
        }
        m_converged = m_nonLin==NULL;
        while( !m_converged )                  // Non Linear Components
        {
            m_converged = true;
            while( m_nonLin ){
                m_nonLin->added = false;
                m_nonLin->voltChanged();
                m_nonLin = m_nonLin->nextChanged;
            }
            if( m_maxNlstp && (m_NLstep++ >= m_maxNlstp) )  // Max iterations reached
            { m_warning = 1; m_converged = true; break; }

            if( m_changedNode ) solveMatrix();
            if( m_state < SIM_RUNNING ) break;    // Loop broken without converging
        }
        if( !m_converged ) return;                // Don't run linear until nonliear converged (Loop broken)

        m_NLstep = 0;
        while( m_voltChanged )
        {
            m_voltChanged->added = false;
            m_voltChanged->voltChanged();
            m_voltChanged = m_voltChanged->nextChanged;
        }
    }
}

void Simulator::resetSim()
{
    m_state    = SIM_STOPPED;
    m_load     = 0;
    m_error    = 0;
    m_warning  = 0;
    m_lastStep = 0;
    m_lastRefT = 0;
    m_circTime = 0;
    m_NLstep   = 0;
    m_realStepsPF = 1;

    CircuitView::self()->setCircTime( 0 );
    clearEventList();
    m_changedNode = NULL;
    m_voltChanged = NULL;
    m_nonLin = NULL;

    CircuitWidget::self()->setMsg( " Stopped ", 1 );
}

void Simulator::createNodes()
{
    //qDebug() <<"\ncreateNodes...\n";
    for( eNode* enode : m_eNodeList )
    {
        enode->clear();
        delete enode;
    }
    m_eNodeList.clear();

    int i = 0;
    QStringList pinList;
    QStringList pins = Circuit::self()->m_pinMap.keys();
    for( QString pinName : pins )
    {
        Pin* pin = Circuit::self()->m_pinMap.value( pinName );
        if( !pin->conPin() ) continue;
        if( pin->isBus() ) continue;
        if( pinList.contains( pinName ) ) continue;

        eNode* node = new eNode( "eNodeSim-"+QString::number(i) );
        i++;
        //qDebug() <<"--------------createNode "<<i<<node->itemId();
        pin->registerPinsW( node );
        pin->registerEnode( node );
        for( ePin* nodePin : node->getEpins() )
        {
            QString pinId = nodePin->getId();//qDebug() <<pinId<<"\t\t\t"<<nodePin->getEnode()->itemId();
            if( !pinList.contains(pinId) ) pinList.append( pinId );
        }
    }
    qDebug() <<"  Created      "<< i << "\teNodes"<<pinList.size()<<"Pins";
}

void Simulator::startSim( bool paused )
{
    resetSim();
    setStepsPerSec( m_stepsPS );
    m_state = SIM_STARTING;

    qDebug() <<"\nStarting Circuit Simulation...\n";

    createNodes();

    qDebug() <<"  Initializing "<< m_elementList.size() << "\teElements";
    for( eElement* el : m_elementList )    // Initialize all Elements
    {                                      // This can create new eNodes
        //qDebug() << "initializing  "<< el->getId();
        el->initialize();
        el->added = false;
    }
    m_changedNode = NULL;

    qDebug() <<"  Initializing "<< m_eNodeList.size()<< "\teNodes";
    for( int i=0; i<m_eNodeList.size(); i++ )         // Initialize eNodes
    {
        eNode* enode = m_eNodeList.at(i);
        enode->setNodeNumber( i+1 );
        enode->initialize();
        //qDebug() << "initializing  "<< enode->itemId();
    }
    for( eElement* el : m_elementList ) el->stamp();

    m_matrix->createMatrix( m_eNodeList );// Initialize Matrix
    if( !m_matrix->solveMatrix() )        // Try to solve matrix, if it fails, stop simulation // m_matrix.printMatrix();
    {
        qDebug() << "Simulator::startSim, Failed to solve Matrix";
        m_error = 1;
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setError( m_errors.value( m_error ) );
        m_state = SIM_ERROR;
        return;
    }
    qDebug() << "\nCircuit Matrix looks good";

    double sps100 = 100*(double)m_stepsPS*m_stepSize/1e12; // Speed %
    double fps = m_stepsPS/m_stepsPF;

    qDebug()  << "\nFPS:   " << fps        << "\t Frames per Sec"
              << "\nSpeed: " << sps100     << "%"
              << "\nStep : " << m_stepSize << "\t picoseconds"
              << "\nSpeed: " << m_stepsPS  << "\t Steps per Sec"
              << "\nStp/F: " << m_stepsPF  << "\t Steps per Frame"
              << "\nNonLi: " << m_maxNlstp << "\t Max Iterations";

    qDebug() << "\n    Simulation Running... \n";

    initTimer();
    if( paused ) pauseSim();
}

void Simulator::stopSim()
{
    stopTimer();
    if( !m_CircuitFuture.isFinished() ) m_CircuitFuture.waitForFinished();

    for( eNode* node  : m_eNodeList  )  node->setVolt( 0 );
    for( eElement* el : m_elementList ) el->initialize();
    for( Updatable* el : m_updateList ) el->updateStep();

    clearEventList();
    m_changedNode = NULL;
}

void Simulator::pauseSim()
{
    if( m_state <= SIM_PAUSED ) return;
    m_oldState = m_state;
    m_state = SIM_PAUSED;

    CircuitWidget::self()->setMsg( " Paused ", 1 );
    qDebug() << "\n    Simulation Paused ";
}

void Simulator::resumeSim()
{
    if( m_state != SIM_PAUSED ) return;
    m_state = m_oldState; // SIM_RUNNING;

    CircuitWidget::self()->setMsg( " Running ", 0 );
    qDebug() << "\n    Resuming Simulation";
}

void Simulator::stopTimer()
{
    if( m_timerId == 0 ) return;
    this->killTimer( m_timerId );
    m_timerId = 0;

    CircuitWidget::self()->setRate( 0, 0 );
    CircuitWidget::self()->setMsg( " Stopped ", 1 );
    Circuit::self()->update();
    qDebug() << "\n    Simulation Stopped ";
    m_state = SIM_STOPPED;
}

void Simulator::initTimer()
{
    if( m_timerId != 0 ) return;
    CircuitWidget::self()->setMsg( " Running ", 0  );
    m_refTime  = m_RefTimer.nsecsElapsed();
    m_loopTime = m_refTime;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
    m_state = SIM_RUNNING;
}

void Simulator::setFps( uint64_t fps )
{
    m_fps = fps;
    setStepsPerSec( m_stepsPS );
}

void Simulator::setStepsPerSec( uint64_t sps )
{
    if( sps < 1 ) sps = 1;

    m_timerTick = 1000/m_fps; // 50 ms default
    m_stepsPS = sps;           // Steps per second
    m_stepsPF = sps/m_fps;     // Steps per frame

    if( sps < m_fps )
    {
        m_stepsPF = 1;
        m_timerTick = 1000/sps; // ms
}   }

double Simulator::NLaccuracy() { return 1/pow(10,m_noLinAcc)/2; }

void Simulator::clearEventList()
{
    m_firstEvent = NULL;
}
void Simulator::addEvent( uint64_t time, eElement* el )
{
    if( m_state < SIM_STARTING ) return;

    if( !el ) /// TODELETE
    {
        qDebug() << "ERROR: Simulator::addEvent NULL event";
        return;
    }
    QString elId = el->getId();

    time += m_circTime;
    eElement* last  = NULL;
    eElement* event = m_firstEvent;

    while( event ){
        if( elId == event->getId() ) // Same event ERROR
        {
            if( time == event->eventTime ) return; // Same Time
            qDebug() << "ERROR: Simulator::addEvent Repeated event"<<elId; /// TODELETE
            qDebug() << event->eventTime << time;
            event = event->nextEvent;
            if( !event ) break;
            if( last ) last->nextEvent = event;
            //return;
        }
        if( time <= event->eventTime ) break; // Insert event here
        last  = event;
        event = event->nextEvent;
    }
    el->eventTime = time;

    if( last ) last->nextEvent = el;
    else       m_firstEvent = el; // List was empty or insert First

    el->nextEvent = event;
}

void Simulator::cancelEvents( eElement* el )
{
    eElement* event = m_firstEvent;
    eElement* last  = NULL;
    eElement* next  = NULL;

    while( event ){
        next = event->nextEvent;
        if( el == event )
        {
            if( last ) last->nextEvent = next;
            else       m_firstEvent = next;

            event->nextEvent = NULL;
            //m_numEvents--;
        }
        else last = event;
        event = next;
}   }

void Simulator::addToEnodeList( eNode* nod )
{ if( !m_eNodeList.contains(nod) ) m_eNodeList.append( nod ); }

void Simulator::remFromEnodeList( eNode* nod, bool del )
{
    if( m_eNodeList.contains( nod ) ) m_eNodeList.removeOne( nod );
    if( del ) delete nod;
}

void Simulator::addToChangedNodes( eNode* nod )
{ nod->nextCH = m_changedNode; m_changedNode = nod; }

void Simulator::addToElementList( eElement* el )
{ if( !m_elementList.contains(el) ) m_elementList.append(el); }

void Simulator::remFromElementList( eElement* el )
{ if( m_elementList.contains(el) ) m_elementList.removeOne(el); }

void Simulator::addToUpdateList( Updatable* el )
{ if( !m_updateList.contains(el) ) m_updateList.append(el); }

void Simulator::remFromUpdateList( Updatable* el )
{ m_updateList.removeOne(el); }

void Simulator::addToChangedFast( eElement* el )
{ el->nextChanged = m_voltChanged; m_voltChanged = el; }

void Simulator::addToNoLinList( eElement* el )
{
    if( el->added ) return;
    el->added = true;
    el->nextChanged = m_nonLin;
    m_nonLin = el;
}

#include "moc_simulator.cpp"
