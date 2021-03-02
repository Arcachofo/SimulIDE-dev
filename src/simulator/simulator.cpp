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

//#include <iostream>

#include "simulator.h"
#include "circuit.h"
#include "matrixsolver.h"
#include "e-element.h"
#include "outpaneltext.h"
#include "mcucomponent.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "baseprocessor.h"


Simulator* Simulator::m_pSelf = 0l;

Simulator::Simulator( QObject* parent )
         : QObject( parent )
{
    m_pSelf = this;

    m_fps = 20;
    m_timerId   = 0;
    m_timerTick = 50;
    m_stepsPS   = 1e6;
    m_stepSize  = 1e6;
    m_noLinAcc  = 5; // Non-Linear accuracy
    m_maxNlstp  = 100000;

    m_errors[0] = "";
    m_errors[1] = "Could not solve Matrix";
    m_errors[2] = "Add Event: NULL free event";
    m_errors[3] = "LAST_SIM_EVENT reached";

    m_warnings[1] = "NonLinear Not Converging";
    m_warnings[100] = "AVR crashed !!!";

    resetSim();

    m_RefTimer.start();
}
Simulator::~Simulator()
{
    m_CircuitFuture.waitForFinished();
}

inline void Simulator::solveMatrix()
{
    while( m_changedNode )
    {
        m_changedNode->stampMatrix();
        m_changedNode = m_changedNode->nextCH;
    }
    if( !m_matrix.solveMatrix() ) // Try to solve matrix, if not stop simulation
    {
        qDebug() << "Simulator::solveMatrix(), Failed to solve Matrix";
        m_error = 1;
    }                                // m_matrix sets the eNode voltages
}

void Simulator::timerEvent( QTimerEvent* e )  //update at m_timerTick rate (50 ms, 20 Hz max)
{
    e->accept();

    if( m_error )
    {
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setError( m_errors.value( m_error ) );
        m_state = SIM_ERROR;
        return;
    }
    if( m_warning > 0 )
    {
        int type = (m_warning < 100)? 1:2;
        CircuitWidget::self()->setMsg( m_warnings.value( m_warning), type );
        m_warning = -10;
    }
    else if( m_warning < 0 )
    {
        if( ++m_warning == 0 ) CircuitWidget::self()->setMsg( " Simulation Running ", 0 );
    }
    //if( m_state < SIM_RUNNING ) return;

    if( !m_CircuitFuture.isFinished() ) // Stop remaining parallel thread
    {
        simState_t state = m_state;
        m_state = SIM_WAITING;
        m_CircuitFuture.waitForFinished();
        m_state = state;
    }
    // Calculate Load
    uint64_t loop = m_loopTime-m_refTime;
    m_load = (m_load+100*loop/(m_timerTick*1e6))/2;

    // Get Real Simulation Speed
    m_refTime = m_RefTimer.nsecsElapsed();
    m_tStep   = m_circTime;

    if( Circuit::self()->animate() )
    {
        Circuit::self()->updateConnectors();
        for( eNode* enode : m_eNodeList ) enode->setVoltChanged( false );
    }

    for( eElement* el : m_updateList ) el->updateStep();

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
    if( m_changedNode ) solveCircuit();  // Solving matrix here save events in updateStep()

    simEvent_t* event = m_eventList.first;
    uint64_t   endRun = m_circTime + m_stepsPF*m_stepSize; // Run upto next Timer event
    uint64_t nextTime;
    //m_maxNlSteps = pow(m_noLinAcc,4);

    while( event )                         // Simulator event loop
    {
        if( event->time > endRun ) break;  // All events for this Timer Tick are done

        nextTime = m_circTime;
        while( m_circTime == nextTime )    // Run all event with same timeStamp
        {
            m_circTime = event->time;
            freeEvent( event );

            if( event->comp ) event->comp->runEvent(); // Run event callback

            event = m_eventList.first;
            if( event ) nextTime = event->time;
            else break;
        }
        solveCircuit();
        if( m_state < SIM_RUNNING ) break;

        /*if( m_changedNode ) solveMatrix();

        while( m_nonLin )                  // Non Linear Components
        {
            while( m_nonLin )
            {
                m_nonLin->added = false;
                m_nonLin->voltChanged();
                m_nonLin = m_nonLin->nextNonLin;
            }
            if( m_changedNode )
            {
                solveMatrix();
                m_NLstep++;
            }
            if( m_state < SIM_RUNNING ) {
                addEvent( 0, NULL ); break; } // Add event so non linear keep running at next timer tick
            if( m_maxNlstp ) { if( m_NLstep >= m_maxNlstp ) { m_warning = 1; break; } }
        }
        if( m_state < SIM_RUNNING ) break; // ???? Keep this at the end for debugger to run 1 step
//        if( (m_maxNlSteps == 0) && (m_noLinSteps>1) )qDebug() << "Simulator::runCircuit m_noLinSteps" << m_noLinSteps;
        m_NLstep = 0;

        while( m_voltChanged )           // Other Components
        {
            m_voltChanged->added = false;
            m_voltChanged->voltChanged();
            m_voltChanged = m_voltChanged->nextChanged;
        }*/
        event = m_eventList.first;
    }
    if( m_state > SIM_WAITING ) m_circTime = endRun;
    m_loopTime = m_RefTimer.nsecsElapsed();
}

void Simulator::solveCircuit()
{
    if( m_changedNode ) solveMatrix();
    while( m_nonLin )                  // Non Linear Components
    {
        while( m_nonLin )
        {
            m_nonLin->added = false;
            m_nonLin->voltChanged();
            m_nonLin = m_nonLin->nextNonLin;
        }
        if( m_changedNode )
        {
            solveMatrix();
            m_NLstep++;
        }
        if( m_state < SIM_RUNNING ) break;
        if( m_maxNlstp ) { if( m_NLstep >= m_maxNlstp ) { m_warning = 1; break; } }
    }
    if( m_state < SIM_RUNNING ) return;
//        if( (m_maxNlSteps == 0) && (m_noLinSteps>1) )qDebug() << "Simulator::runCircuit m_noLinSteps" << m_noLinSteps;
    m_NLstep = 0;
    while( m_voltChanged )
    {
        m_voltChanged->added = false;
        m_voltChanged->voltChanged();
        m_voltChanged = m_voltChanged->nextChanged;
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

    CircuitView::self()->setCircTime( 0 );
    clearEventList();
    m_changedNode = NULL;
    m_voltChanged = NULL;
    m_nonLin = NULL;

    CircuitWidget::self()->setMsg( " Simulation Stopped ", 1 );
}

void Simulator::startSim( bool paused )
{
    resetSim();
    setStepsPerSec( m_stepsPS );
    m_state = SIM_STARTING;
    addEvent( 0, NULL );

    qDebug() <<"\nStarting Circuit Simulation...\n";

    for( eNode* busNode : m_eNodeBusList ) busNode->initialize(); // Initialize Bus Nodes

    qDebug() <<"  Initializing "<< m_elementList.size() << "\teElements";
    for( eElement* el : m_elementList )    // Initialize all Elements
    {
        //qDebug() << "initializing  "<< el->getId();
        el->initialize();
        el->added = false;
    }

    qDebug() <<"  Initializing "<< m_eNodeBusList.size() << "\tBuses";
    for( eNode* busNode : m_eNodeBusList ) busNode->createBus(); // Create Buses

    m_changedNode = NULL;
    for( eElement* el : m_elementList ) el->attach(); // Connect Elements with internal circuits.

    qDebug() <<"  Initializing "<< m_eNodeList.size()<< "\teNodes";
    for( int i=0; i<m_eNodeList.size(); i++ )         // Initialize eNodes
    {
        eNode* enode = m_eNodeList.at(i);
        enode->setNodeNumber( i+1 );
        enode->initialize();
    }
    for( eElement* el : m_elementList ) el->stamp();

    m_matrix.createMatrix( m_eNodeList );// Initialize Matrix
    if( !m_matrix.solveMatrix() )        // Try to solve matrix, if it fails, stop simulation // m_matrix.printMatrix();
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
              << "\nSpeed: " << m_stepsPS  << "\t Steps per Sec"
              << "\nStp/F: " << m_stepsPF  << "\t Steps per Frame"
              << "\nNonLi: " << m_maxNlstp << "\t Max Iterations"
              << "\nStep : " << m_stepSize << "\t picoseconds";

    qDebug() << "\n    Simulation Running... \n";

    initTimer();
    if( paused ) pauseSim();
}

void Simulator::stopSim()
{
    stopTimer();
    m_CircuitFuture.waitForFinished();

    for( eNode* node  : m_eNodeList  )  node->setVolt( 0 );
    for( eElement* el : m_elementList ) el->initialize();
    for( eElement* el : m_updateList )  el->updateStep();

    clearEventList();
    m_changedNode = NULL;
}

void Simulator::pauseSim()
{
    m_state = SIM_PAUSED;

    CircuitWidget::self()->setMsg( " Simulation Paused ", 1 );
    qDebug() << "\n    Simulation Paused ";
}

void Simulator::resumeSim()
{
    m_state = SIM_RUNNING;

    CircuitWidget::self()->setMsg( " Simulation Running ", 0 );
    qDebug() << "\n    Resuming Simulation";
}

void Simulator::stopTimer()
{
    if( m_timerId == 0 ) return;
    this->killTimer( m_timerId );
    m_timerId = 0;

    CircuitWidget::self()->setRate( 0, 0 );
    CircuitWidget::self()->setMsg( " Simulation Stopped ", 1 );
    Circuit::self()->update();
    qDebug() << "\n    Simulation Stopped ";
    m_state = SIM_STOPPED;
}

void Simulator::initTimer()
{
    if( m_timerId != 0 ) return;
    CircuitWidget::self()->setMsg( " Simulation Running ", 0  );
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

    m_timerTick  = 1000/m_fps; // 50 ms default

    m_stepsPS = sps;           // Steps per second
    m_stepsPF = sps/m_fps;     // Steps per frame

    if( sps < m_fps )
    {
        m_stepsPF = 1;
        m_timerTick = 1000/sps; // ms
    }

    if( this->isRunning() )
    {
        pauseSim();
        emit rateChanged();
        resumeSim();
    }
}

void  Simulator::setNoLinAcc( int ac )
{
    bool running = isRunning();
    if( running ) pauseSim();

    if     ( ac < 1 )  ac = 1;
    else if( ac > 14 ) ac = 14;
    m_noLinAcc = ac;

    if( running ) resumeSim();
}

void Simulator::clearEventList()
{
    for( int i=0; i<LAST_SIM_EVENT; i++ )
    {
        m_eventList.events[i].next = &(m_eventList.events[i+1]);
        m_eventList.events[i].comp = 0l;
        m_eventList.events[i].time = 0;
    }
    m_eventList.free = &(m_eventList.events[0]);
    m_eventList.first = NULL;
    m_numEvents = 0;
}

void Simulator::addEvent( uint64_t time, eElement* comp )
{
    if( m_state < SIM_STARTING ) return;

    time += m_circTime;
    simEvent_t* last  = NULL;
    simEvent_t* event = m_eventList.first;
    simEvent_t* new_event = m_eventList.free;

    while( event )
    {
        if( time <= event->time)
        {
            if( comp == NULL
            && event->comp == NULL
            && time == event->time ) return;
            break;
        }
        last  = event;
        event = event->next;
    }
    m_eventList.free = new_event->next;

    new_event->time = time;
    new_event->comp = comp;

    if( last ) last->next = new_event;
    else       m_eventList.first = new_event; // List was empty or insert First

    new_event->next = event;
    if( ++m_numEvents >= LAST_SIM_EVENT ) { m_error = 3; return; }
}

void Simulator::cancelEvents( eElement* comp )
{
    simEvent_t* event = m_eventList.first;
    simEvent_t* last  = 0l;
    simEvent_t* next  = 0l;

    while( event )
    {
        next = event->next;
        if( comp == event->comp )
        {
            if( last ) last->next  = next;
            else       m_eventList.first = next;

            event->next = m_eventList.free;
            m_eventList.free = event;
            m_numEvents--;
        }
        else last = event;
        event = next;
    }
}

inline void Simulator::freeEvent( simEvent_t* event )
{
    m_eventList.first = event->next;
    event->next = m_eventList.free;
    m_eventList.free = event;
    m_numEvents--;
}

void Simulator::addToEnodeBusList( eNode* nod )
{ if( !m_eNodeBusList.contains(nod) ) m_eNodeBusList.append( nod ); }

void Simulator::remFromEnodeBusList( eNode* nod, bool del )
{
    if( m_eNodeBusList.contains(nod) ) m_eNodeBusList.removeOne( nod );
    if( del ) delete nod;
}

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

void Simulator::addToUpdateList( eElement* el )
{ if( !m_updateList.contains(el) ) m_updateList.append(el); }

void Simulator::remFromUpdateList( eElement* el )
{ m_updateList.removeOne(el); }

void Simulator::addToChangedFast( eElement* el )
{ el->nextChanged = m_voltChanged; m_voltChanged = el; }

void Simulator::addToNoLinList( eElement* el )
{ el->nextNonLin = m_nonLin; m_nonLin = el; }

#include "moc_simulator.cpp"

