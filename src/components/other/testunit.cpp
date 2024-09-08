/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "testunit.h"
#include "itemlibrary.h"
#include "truthtable.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "batchtest.h"
#include "iopin.h"

#include "stringprop.h"

#define tr(str) simulideTr("TestUnit",str)

Component* TestUnit::construct( QString type, QString id )
{ return new TestUnit( type, id ); }

LibraryItem* TestUnit::libraryItem()
{
    return new LibraryItem(
        tr("Test Unit"),
        "Other",
        "bug.png",
        "TestUnit",
        TestUnit::construct );
}

TestUnit::TestUnit( QString type, QString id )
        : IoComponent( type, id )
        , eElement( id )
{
    m_width  = 4;
    m_height = 4;

    m_testing = false;

    m_interval = 100*1e3; // 100 ns
    m_truthTable = nullptr;

    setInputs("O");
    setOutputs("I0,I1");

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new StrProp<TestUnit>("Inputs", tr("Inputs"),""
                             , this, &TestUnit::inputs, &TestUnit::setInputs, propNoCopy ),

        new StrProp<TestUnit>("Outputs", tr("Outputs"),""
                             , this, &TestUnit::outputs, &TestUnit::setOutputs, propNoCopy ),

        new StrProp<TestUnit>("Truth", "Truth",""
                             , this, &TestUnit::truth, &TestUnit::setTruth, propHidden ),
    },0} );
}
TestUnit::~TestUnit()
{
    if( m_truthTable )
    {
        //m_truthTable->clear();
        delete m_truthTable;
    }
}

void TestUnit::stamp()
{
    IoComponent::initState();

    m_read = false;
    m_changed = false;

    updtData();

    Simulator::self()->addEvent( m_interval, this );
    if( BatchTest::isRunning() ) BatchTest::addTestUnit( this );
}

void TestUnit::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    createTable();
}

void TestUnit::runEvent()
{
    if( m_read ) {
        m_read = false;

        uint inputVal = 0;
        for( uint i=0; i<m_inPin.size(); ++i )
            if( m_inPin[i]->getInpState() ) inputVal |= 1<<i;

        m_samples[m_outValue] = inputVal;

        if( ++m_outValue < (uint)m_steps )
            Simulator::self()->addEvent( m_interval, this );
        else m_changed = true;
    }else{
        m_read = true;
        for( uint i=0; i<m_outPin.size(); ++i )
        {
            bool state = m_outValue & (1<<i);
            m_outPin[i]->setOutState( state );
        }
        Simulator::self()->addEvent( m_interval, this );
    }
}

void TestUnit::runTest()
{
    CircuitWidget::self()->powerCircOff();
    CircuitWidget::self()->powerCircOn();
}

void TestUnit::createTable()
{
    if( !m_truthTable )
        m_truthTable = new TruthTable( this, CircuitView::self() );

    bool testOk = m_truthTable->setup( m_inputStr, m_outputStr, &m_samples, &m_truthT );
    m_testing = !(m_truthT.size() == 0);
    if( m_testing ) {
        if( BatchTest::isRunning() ) BatchTest::testCompleted( this, testOk );
    }
    else m_truthT = m_samples; // save samples as truth

    m_truthTable->show();
}

void TestUnit::save()
{
    m_truthT = m_samples;
    m_truthTable->setup( m_inputStr, m_outputStr, &m_samples, &m_truthT );
}

void TestUnit::loadTest()
{
}

QString TestUnit::truth()
{
    QString truthStr;
    for( uint i=0; i<m_truthT.size(); ++i )
    {
        truthStr += QString::number( m_truthT[i],16 )+",";
    }
    return truthStr;
}

void TestUnit::setTruth( QString t )
{
    QStringList truthList = t.split(",");

    bool ok;
    m_truthT.clear();
    for( QString valStr : truthList )
    {
        if( !valStr.isEmpty() )
            m_truthT.push_back( valStr.toUInt( &ok, 16 ) );
    }
}

void TestUnit::setInputs( QString i )
{
    m_inputStr = i;
    QStringList inputList = i.split(",");

    int size = inputList.size();
    IoComponent::setNumOuts( size, "I" );

    for( int i=0; i<size; ++i )
        m_outPin[i]->setLabelText( inputList.at(i) );

    updtOutPins();
    updtData();
}

void TestUnit::setOutputs( QString o )
{
    m_outputStr = o;
    QStringList outputList = o.split(",");

    int size = outputList.size();
    IoComponent::setNumInps( size, "O" );

    for( int i=0; i<size; ++i )
        m_inPin[i]->setLabelText( outputList.at(i) );

    updtInPins();
    updtData();
}

void TestUnit::updtData()
{
    m_steps = pow( 2, m_outPin.size() ); // Output pins are the inputs of the tested device
    m_samples.clear();
    m_samples.resize( m_steps );

    m_truthT.clear();
    m_truthT.resize( m_steps );
}
