/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <qtconcurrentrun.h>
#include <QHash>
#include <math.h>

#include "simulator.h"
#include "editorwindow.h"
#include "circuit.h"
#include "matrixsolver.h"
#include "updatable.h"
#include "outpaneltext.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "circmatrix.h"
#include "e-element.h"
#include "socket.h"

Simulator* Simulator::m_pSelf = NULL;

Simulator::Simulator( QObject* parent )
         : QObject( parent )
{
    m_pSelf = this;

    m_matrix = new CircMatrix();

    m_fps = 20;
    m_timerId   = 0;
    m_timerTick = 50;   // 50 ms default
    m_psPerSec  = 1e12;
    m_stepSize  = 1e6;
    m_stepsPS   = 1e6;
    m_reactStep = 1e6;
    m_maxNlstp  = 100000;
    m_slopeSteps = 0;

    m_errors[0] = "";
    //m_errors[1] = "Could not solve Matrix";
    m_errors[2] = "Add Event: NULL free event";
    m_errors[3] = "LAST_SIM_EVENT reached";

    m_warnings[1] = "NonLinear Not Converging";
    m_warnings[2] = "Probably Circuit Error";  // Warning if matrix diagonal element = 0.
    m_warnings[100] = "AVR crashed !!!";

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
    if( !m_matrix->solveMatrix() ) // m_matrix sets the eNode voltages
        m_warning = 2;             // Warning if diagonal element = 0.
}

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
    { if( ++m_warning == 0 ) CircuitWidget::self()->setMsg( " "+tr("Running")+" ", 0 ); }

    if( !m_CircuitFuture.isFinished() ) // Stop remaining parallel thread
    {
        simState_t state = m_state;
        m_state = SIM_WAITING;
        m_CircuitFuture.waitForFinished();
        m_state = state;
    }

    if( Circuit::self()->animate() )
    {
        uint updateRate = m_fps/5;
        if( updateRate < 1 ) updateRate = 1;
        if( ++m_updtCnt >= updateRate )     // Animate at FPS/5 with minimum 4 FPS
        {
            m_updtCnt = 0;
            Circuit::self()->updateConnectors();
            for( eNode* enode : m_eNodeList ) enode->setVoltChanged( false );
        }
    }
    for( Updatable* el : m_updateList ) el->updateStep();
    EditorWindow::self()->outPane()->updateStep(); // OutPanel in Editor can be created before this simulator.

    // Calculate Load
    uint64_t loop = 0;
    if( m_loopTime > m_refTime ) loop = m_loopTime-m_refTime;
    m_load = (m_load+100*loop/((double)m_timerTick*1e6))/2;

    // Get Real Simulation Speed
    m_realPsPF = m_circTime-m_tStep;
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
    CircuitWidget::self()->setCircTime( m_tStep );
}

void Simulator::runCircuit()
{
    solveCircuit(); // Solve any pending changes
    if( m_state < SIM_RUNNING ) return;

    eElement* event = m_firstEvent;
    uint64_t endRun = m_circTime + m_psPF; // Run upto next Timer event
    uint64_t nextTime;

    while( event )                              // Simulator event loop
    {
        if( event->eventTime > endRun ) break;  // All events for this Timer Tick are done

        nextTime = m_circTime;
        while( m_circTime == nextTime )         // Run all event with same timeStamp
        {
            m_circTime = event->eventTime;
            m_firstEvent = event->nextEvent;    // free Event
            event->nextEvent = NULL;
            event->eventTime = 0;
            event->runEvent();                  // Run event callback
            event = m_firstEvent;
            if( event ) nextTime = event->eventTime;
            else break;
        }
        solveCircuit();
        if( m_state < SIM_RUNNING ) break;
        event = m_firstEvent;               // m_firstEvent can be an event added at solveCircuit()
    }
    if( m_state > SIM_WAITING ) m_circTime = endRun;
    m_loopTime = m_RefTimer.nsecsElapsed();
}

void Simulator::solveCircuit()
{
    while( m_changedNode || m_nonLinear || !m_converged ) // Also Proccess changes gererated in voltChanged()
    {
        if( m_changedNode ) solveMatrix();

        if( m_converged ) m_converged = m_nonLinear==NULL;
        while( !m_converged )              // Non Linear Components
        {
            m_converged = true;
            while( m_nonLinear ){
                m_nonLinear->added = false;
                m_nonLinear->voltChanged();
                m_nonLinear = m_nonLinear->nextChanged;
            }
            if( m_maxNlstp && (m_NLstep++ >= m_maxNlstp) ) { m_warning = 1; return; } // Max iterations reached
            if( m_state < SIM_RUNNING ){ m_converged = false; break;}    // Loop broken without converging
            if( m_changedNode ) solveMatrix();
        }
        if( !m_converged ) return; // Don't run linear until nonliear converged (Loop broken)

        m_NLstep = 0;
        while( m_voltChanged )
        {
            m_voltChanged->added = false;
            m_voltChanged->voltChanged();
            m_voltChanged = m_voltChanged->nextChanged;
        }
        if( m_state < SIM_RUNNING ) break;    // Loop broken without converging
    }
}

void Simulator::resetSim()
{
    m_state    = SIM_STOPPED;
    m_load     = 0;
    m_error    = 0;
    m_warning  = 0;
    m_lastStep = 0;
    m_tStep    = 0;
    m_lastRefT = 0;
    m_circTime = 1;
    m_NLstep   = 0;
    ///m_pauseCirc = false;
    m_realPsPF = 1;

    CircuitWidget::self()->setCircTime( 0 );
    clearEventList();
    m_changedNode = NULL;
    m_voltChanged = NULL;
    m_nonLinear = NULL;

    CircuitWidget::self()->setMsg( " "+tr("Stopped")+" ", 1 );
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
    QStringList pinNames = Circuit::self()->m_pinMap.keys();
    pinNames.sort();
    for( QString pinName : pinNames )
    {
        Pin* pin = Circuit::self()->m_pinMap.value( pinName );
        if( !pin ) continue;
        if( pinList.contains( pinName ) ) continue;
        if( !pin->conPin() ) continue;
        if( pin->isBus() ) continue;
        if( pinName.startsWith("Node") ) continue;

        eNode* node = new eNode( "eNodeSim-"+QString::number(i) );
        i++;
        //qDebug() <<"--------------createNode "<<i<<node->itemId();
        pin->registerPinsW( node );
        pin->registerEnode( node );
        for( ePin* nodePin : node->getEpins() )
        {
            QString pinId = nodePin->getId();//qDebug() <<pinId<<"\t\t\t"<<nodePin->getEnode()->itemId();
            if( pinId.startsWith("Node") ) continue;
            if( !pinList.contains(pinId) ) pinList.append( pinId );
        }
    }
    qDebug() <<"  Created      "<< i << "\teNodes"<<pinList.size()<<"Pins";
}

void Simulator::startSim( bool paused )
{
    resetSim();
    setPsPerSec( m_psPerSec );
    m_debug = paused;
    m_state = SIM_STARTING;

    qDebug() <<"\nStarting Circuit Simulation...\n";

    for( Socket* sock : m_socketList ) sock->updatePins( true );

    createNodes();

    qDebug() <<"  Initializing "<< m_elementList.size() << "\teElements";
    for( eElement* el : m_elementList )    // Initialize all Elements
    {                                      // This can create new eNodes
        //qDebug() << "initializing  "<< el->getId();
        el->eventTime = 0;
        el->initialize();
        el->added = false;
    }

    qDebug() <<"  Initializing "<< m_eNodeList.size()<< "\teNodes";
    for( int i=0; i<m_eNodeList.size(); i++ )         // Initialize eNodes
    {
        eNode* enode = m_eNodeList.at(i);
        enode->setNodeNumber( i+1 );
        enode->initialize();
        //qDebug() << "initializing  "<< enode->itemId();
    }
    for( eElement* el : m_elementList ) el->stamp();

    m_matrix->createMatrix( m_eNodeList );

    qDebug() << "\nCircuit Matrix looks good";

    double sps100 = 100*(double)m_psPerSec/1e12; // Speed %

    qDebug()  << "\nFPS:  " << m_fps      << "\tFrames per Sec"
              << "\nSpeed:" << sps100     << "%"
//              << "\nStep: " << m_stepSize << "\tpicoseconds"
              << "\nSpeed:" << m_psPerSec << "\tps per Sec"
              << "\nps/Fr:" << m_psPF     << "\tps per Frame"
              << "\nNonLi:" << m_maxNlstp << "\tMax Iterations";

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

void Simulator::pauseSim() // Only pause simulation, don't update UI
{
    if( m_state <= SIM_PAUSED ) return;
    m_oldState = m_state;
    m_state = SIM_PAUSED;
}

void Simulator::resumeSim()
{
    if( m_state != SIM_PAUSED ) return;
    m_state = m_oldState;
}

void Simulator::stopTimer()
{
    if( m_timerId == 0 ) return;
    this->killTimer( m_timerId );
    m_timerId = 0;

    CircuitWidget::self()->setRate( 0, 0 );
    CircuitWidget::self()->setMsg( " "+tr("Stopped")+" ", 1 );
    Circuit::self()->update();
    qDebug() << "\n    Simulation Stopped ";
    m_state = SIM_STOPPED;
}

void Simulator::initTimer()
{
    if( m_timerId != 0 ) return;
    CircuitWidget::self()->setMsg( " "+tr("Running")+" ", 0 );
    m_refTime  = m_RefTimer.nsecsElapsed();
    m_loopTime = m_refTime;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
    m_state = SIM_RUNNING;
}

void Simulator::setFps( uint64_t fps )
{
    m_fps = fps;
    setPsPerSec( m_psPerSec );
}

void Simulator::setStepsPerSec( uint64_t sps ) // Only used by Load Circuit (old circuits)
{
    if( sps < 1 ) sps = 1;

    m_stepsPS = sps;           // Steps per second
    setPsPerSec( m_stepsPS*m_stepSize );
}

void Simulator::setPsPerSec( uint64_t psPs )
{
    if( psPs < 1 ) psPs = 1;

    m_psPerSec = psPs;    // picosecond/second
    m_psPF = psPs/m_fps;  // picosecond/frame

    uint64_t fps = m_fps;
    if( m_psPF == 0 ) // We must lower fps to get at least 1 ps per frame
    {
        m_psPF = 1;
        fps = psPs;
    }
    m_timerTick = 1000/fps;  // in ms

    CircuitWidget::self()->setTargetSpeed( 100*m_psPerSec/1e12 );
}

void Simulator::clearEventList()
{
    m_firstEvent = NULL;
}
void Simulator::addEvent( uint64_t time, eElement* el )
{
    if( m_state < SIM_STARTING ) return;

    if( el->eventTime )
    { qDebug() << "Warning: Simulator::addEvent Repeated event"<<el->getId(); return; }

    time += m_circTime;
    eElement* last  = NULL;
    eElement* event = m_firstEvent;

    while( event ){
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
    if( el->eventTime == 0 ) return;
    eElement* event = m_firstEvent;
    eElement* last  = NULL;
    eElement* next  = NULL;
    el->eventTime = 0;

    while( event ){
        next = event->nextEvent;
        if( el == event )
        {
            if( last ) last->nextEvent = next;
            else       m_firstEvent = next;
            event->nextEvent = NULL;
        }
        else last = event;
        event = next;
}   }

void Simulator::addToEnodeList( eNode* nod )
{ if( !m_eNodeList.contains(nod) ) m_eNodeList.append( nod ); }

void Simulator::addToElementList( eElement* el )
{ if( !m_elementList.contains(el) ) m_elementList.append(el); }

void Simulator::remFromElementList( eElement* el )
{ m_elementList.removeOne(el); }

void Simulator::addToUpdateList( Updatable* el )
{ if( !m_updateList.contains(el) ) m_updateList.append(el); }

void Simulator::remFromUpdateList( Updatable* el )
{ m_updateList.removeOne(el); }

void Simulator::addToSocketList( Socket* el )
{ if( !m_socketList.contains(el) ) m_socketList.append(el); }

void Simulator::remFromSocketList( Socket* el )
{ m_socketList.removeOne(el); }

#include "moc_simulator.cpp"
