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

#include <iostream>

#include "circuit.h"
#include "matrixsolver.h"
#include "e-element.h"
#include "outpaneltext.h"
#include "mcucomponent.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "baseprocessor.h"
#include "simulator.h"

Simulator* Simulator::m_pSelf = 0l;

Simulator::Simulator( QObject* parent )
         : QObject( parent )
{
    m_pSelf = this;

    m_timerId   = 0;
    m_timerTick = 50;
    m_stepsReac = 10;
    m_stepsPS   = 1e6;
    m_noLinAcc  = 5; // Non-Linear accuracy

    m_errors[0] = " ";
    m_errors[1] = " Could not solve Matrix";
    m_errors[2] = " Add Event: NULL free event";
    m_errors[3] = " Add Event: LAST_SIM_EVENT reached";

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
        std::cout << "Simulator::solveMatrix(), Failed to solve Matrix" << std::endl;
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
    if( m_state < SIM_RUNNING ) return;

    if( !m_CircuitFuture.isFinished() ) // Stop remaining parallel thread
    {
        m_state = SIM_WAITING;
        m_CircuitFuture.waitForFinished();
        m_state = SIM_RUNNING;
    }
    // Calculate Load
    uint64_t loop = m_loopTime-m_refTime;
    m_load = (m_load+100*loop/(m_timerTick*1e6))/2;

    // Get Real Simulation Speed
    m_refTime = m_RefTimer.nsecsElapsed();
    m_tStep   = m_circTime;

    runGraphicStep1();
    // Run Circuit in parallel thread
    m_CircuitFuture = QtConcurrent::run( this, &Simulator::runCircuit ); // Run Circuit in a parallel thread

    runGraphicStep2();
}

void Simulator::runGraphicStep()
{
    runGraphicStep1();
    runGraphicStep2();
}

void Simulator::runGraphicStep1()
{
    for( eElement* el : m_updateList ) el->updateStep();
}

void Simulator::runGraphicStep2()
{
    if( m_state == SIM_DEBUGG ) m_tStep = m_circTime;        // Debugger driving, not timer

    if( Circuit::self()->animate() )
    {
        Circuit::self()->updateConnectors();
        for( eNode* enode : m_eNodeList ) enode->setVoltChanged( false );
    }

    uint64_t deltaRefTime = m_refTime-m_lastRefT;
    if( deltaRefTime >= 1e9 )               // We want steps per 1 Sec = 1e9 ns
    {
        uint64_t stepsPerSec = (m_tStep-m_lastStep)*10/deltaRefTime;
        CircuitWidget::self()->setRate( stepsPerSec, m_load );
        m_lastStep = m_tStep;
        m_lastRefT = m_refTime;
    }
    CircuitView::self()->setCircTime( m_tStep/1e6 );
}

void Simulator::runCircuit()
{
    simEvent_t* event = m_eventList.first;
    uint64_t   endRun = m_circTime + m_stepsPF*1e6; // Run upto next Timer event
    eElement*   comp;

    while( event )                         // Simulator event loop
    {
        if( event->time > endRun ) break;  // All events for this Timer Tick are done
        m_circTime = event->time;
        freeEvent( event );

        comp = event->comp;
        if( comp ) comp->runEvent();       // Run event callback

        if( m_changedNode ) solveMatrix();

        while( m_nonLin )      // Non Linear Components
        {
            while( m_nonLin )
            {
                m_nonLin->added = false;
                m_nonLin->voltChanged();
                m_nonLin = m_nonLin->nextNonLin;
            }
            if( m_changedNode ) solveMatrix();

            if( m_state < SIM_RUNNING ) { addEvent( 0, 0l ); break; } // Add event so non linear keep running at next timer tick
        }
        if( m_state < SIM_RUNNING ) break; // ???? Keep this at the end for debugger to run 1 step

        while( m_voltChanged )  // Linear Components
        {
            m_voltChanged->added = false;
            m_voltChanged->voltChanged();
            m_voltChanged = m_voltChanged->nextChanged;
        }

        event = m_eventList.first;
    }
    if( m_state > SIM_STARTING ) m_circTime = endRun;
    m_loopTime = m_RefTimer.nsecsElapsed();
}

void Simulator::resetSim()
{
    m_state    = SIM_STOPPED;
    m_load     = 0;
    m_error    = 0;
    m_lastStep = 0;
    m_lastRefT = 0;
    m_circTime = 0;

    CircuitView::self()->setCircTime( 0 );
    clearEventList();
    m_changedFast.clear();
    m_nonLinear.clear();
    m_changedNode = NULL;
    m_voltChanged = NULL;
    m_nonLin = NULL;
}

void Simulator::startSim()
{
    /*if( m_state == SIM_DEBUGG ) // Debugger Controllig Simulation
    {
        debug( false );
        emit resumeDebug();
        return;
    }*/
    resetSim();
    simuRateChanged( m_stepsPS );
    m_state = SIM_STARTING;
    addEvent( 0, NULL );

    std::cout <<"\nStarting Circuit Simulation...\n"<< std::endl;

    for( eNode* busNode : m_eNodeBusList ) busNode->initialize(); // Initialize Bus Nodes

    std::cout <<"  Initializing "<< m_elementList.size() << "\teElements"<< std::endl;
    for( eElement* el : m_elementList )    // Initialize all Elements
    {
        //qDebug() << "initializing  "<< el->getId();
        el->initialize();
        el->added = false;
    }

    std::cout <<"  Initializing "<< m_eNodeBusList.size() << "\tBuses"<< std::endl;
    for( eNode* busNode : m_eNodeBusList ) busNode->createBus(); // Create Buses

    m_changedNode = NULL;
    for( eElement* el : m_elementList ) el->attach(); // Connect Elements with internal circuits.

    std::cout <<"  Initializing "<< m_eNodeList.size()<< "\teNodes"<< std::endl;
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
        std::cout << "Simulator::startSim, Failed to solve Matrix"
                  <<  std::endl;
        m_error = 1;
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setError( m_errors.value( m_error ) );
        m_state = SIM_ERROR;
        return;
    }
    std::cout << "\nCircuit Matrix looks good" <<  std::endl;

    m_state = SIM_RUNNING;

    std::cout << "\n    Simulation Running... \n"<<std::endl;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
}

void Simulator::stopSim()
{
    stopTimer();
    m_state = SIM_STOPPED;
    m_CircuitFuture.waitForFinished();

    for( eNode* node  : m_eNodeList  )  node->setVolt( 0 );
    for( eElement* el : m_elementList ) el->initialize();
    for( eElement* el : m_updateList )  el->updateStep();

    clearEventList();
    clearNodeList();

    CircuitWidget::self()->setRate( 0, 0 );
    Circuit::self()->update();

    std::cout << "\n    Simulation Stopped \n" << std::endl;
}

void Simulator::pauseSim()
{
    //emit pauseDebug();
    stopTimer();
    m_state = SIM_PAUSED;
    //m_CircuitFuture.waitForFinished();
    std::cout << "\n    Simulation Paused \n" << std::endl;
}

void Simulator::resumeSim()
{
    m_state = SIM_RUNNING;
    resumeTimer();

    //emit resumeDebug();
    //if( m_debugging ) return;

    std::cout << "\n    Resuming Simulation\n" << std::endl;
}

void Simulator::stopTimer()
{
    if( m_timerId == 0 ) return;
    this->killTimer( m_timerId );
    m_timerId = 0;
}

void Simulator::resumeTimer() // Used by code editor
{
    if( m_timerId != 0 ) return;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
}

void Simulator::simuRateChanged( uint64_t rate )
{
    if( rate < 1 ) rate = 1;

    m_timerTick  = 50;
    uint64_t fps = 1000/m_timerTick;

    m_stepsPF = rate/fps;

    if( rate < fps )
    {
        fps = rate;
        m_stepsPF = 1;
        m_timerTick = 1000/rate;
    }
    m_stepsPS = m_stepsPF*fps;

    if( this->isRunning() )
    {
        pauseSim();
        emit rateChanged();
        resumeSim();
    }

    double sps100 = 100*(double)m_stepsPS/1e6;

    std::cout << "\nFPS:   " << fps
              << "\nus/F:  " << m_stepsPF   << "\t Simul. us"
              << std::endl
              << "\nSpeed: " << sps100      << "%"
              << "\nSpeed: " << m_stepsPS   << "\t us Per Sec"
              << "\nReact: " << m_stepsReac << "\t Simul. Steps"
              << std::endl
              << std::endl;
}

void Simulator::debug( bool run )
{
    /*if( run )
    {
        //m_debugging = false;
        runContinuous();
    }
    else
    {
        startSim();
        //m_isrunning = false;
        //m_debugging = true;
        std::cout << "\n    Debugger Controllig Simulation... \n"<<std::endl;
    }*/
}

void Simulator::stopDebug()
{
    //m_debugging = false;
    //stopSim();
}

void Simulator::setReaClock( int value )
{
    bool running = isRunning();
    if( running ) pauseSim();

    if     ( value < 1  )  value = 1;
    else if( value > 100 ) value = 100;

    m_stepsReac = value;

    if( running ) resumeSim();
}

void  Simulator::setNoLinAcc( int ac )
{
    bool running = isRunning();
    if( running ) pauseSim();

    if     ( ac < 3 )  ac = 3;
    else if( ac > 14 ) ac = 14;
    m_noLinAcc = ac;

    if( running ) resumeSim();
}

void Simulator::clearNodeList()
{
    m_changedNode = NULL;
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
    m_eventList.first = 0l;
}

void Simulator::addEvent( uint64_t time, eElement* comp )
{
    if( m_state < SIM_STARTING ) return;
    time += m_circTime;
    simEvent_t* last  = 0l;
    simEvent_t* event = m_eventList.first;
    simEvent_t* new_event = m_eventList.free;

    //if( new_event == 0 ) { m_error = 2; return; }

    //int i = 0;
    while( event )
    {
        if( time <= event->time ) break;
        last  = event;
        event = event->next;
        //if( ++i > LAST_SIM_EVENT ) { m_error = 3; return; }
    }
    m_eventList.free = new_event->next;

    new_event->time = time;
    new_event->comp = comp;

    if( last ) last->next = new_event;
    else       m_eventList.first = new_event; // List was empty or insert First

    new_event->next = event;
}

void Simulator::cancelEvents( eElement* comp )
{
    simEvent_t* event = m_eventList.first;
    simEvent_t* last  = 0l;

    int i = 0;
    while( event )
    {
        if( comp == event->comp )
        {
            if( last ) last->next  = event->next;
            event->next = m_eventList.free;
            m_eventList.free = event;
        }
        last  = event;
        event = event->next;
        if( ++i > LAST_SIM_EVENT ) { m_error = 3; return; }
    }
}

inline void Simulator::freeEvent( simEvent_t* event )
{
    m_eventList.first = event->next;
    event->next = m_eventList.free;
    m_eventList.free = event;
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

