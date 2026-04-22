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
#include "doubleprop.h"

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

    m_steps = 0;

    m_period = 1e-7; // 100 ns
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

    addPropGroup( { tr("Test"), {
        new DoubProp<TestUnit>("Period",tr("Period"),"ns"
                              , this, &TestUnit::period, &TestUnit::setPeriod,0  ),

        //new BoolProp<TestUnit>("DoTest",tr("Do Test"),""
        //                      , this, &TestUnit::doTest, &TestUnit::setDoTest,0 ),
    }, 0 } );
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

    resizeVectors();

    if( BatchTest::isRunning() )
    {
        createTable();
        BatchTest::addTestUnit( this );
        Simulator::self()->addEvent( m_period*1e12/2, this );
    }
}

void TestUnit::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    CircuitWidget::self()->powerCircOff();
    bool testOk = m_truthTable->checkThruth( &m_samples );
    if( !BatchTest::isRunning() ) BatchTest::testCompleted( this, testOk );
}

void TestUnit::runEvent() // Running test
{
    if( m_read ) {
        m_read = false;

        uint inputVal = 0;
        for( uint i=0; i<m_inpPin.size(); ++i )
            if( m_inpPin[i]->getInpState() ) inputVal |= 1<<i;

        m_samples[m_outValue] = inputVal;

        if( ++m_outValue < (uint)m_steps )
            Simulator::self()->addEvent( m_period*1e12/2, this );
        else m_changed = true;
    }else{
        m_read = true;
        for( uint i=0; i<m_outPin.size(); ++i )
        {
            bool state = m_outValue & (1<<i);
            m_outPin[i]->setOutState( state );
        }
        Simulator::self()->addEvent( m_period*1e12/2, this );
    }
}

void TestUnit::runTest() // Run test fron TruthTable
{
    CircuitWidget::self()->powerCircOff();
    CircuitWidget::self()->powerCircOn();
    Simulator::self()->addEvent( m_period*1e12/2, this );
}

void TestUnit::createTable()
{
    if( !m_truthTable )
        m_truthTable = new TruthTable( this, CircuitView::self() );

    m_truthTable->setup( m_inputStr, m_outputStr, m_truthT );
    m_truthTable->show();
}

void TestUnit::save( std::vector<uint> outValues ) // Save current samples as truth
{
    m_truthT = outValues;
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
    uint size = m_truthT.size();
    m_truthT.clear();
    for( QString valStr : truthList )
    {
        if( valStr.isEmpty() ) continue;
        m_truthT.push_back( valStr.toUInt( &ok, 16 ) );
        if( m_truthT.size() == size ) break;
    }
}

void TestUnit::setInputs( QString i )
{
    if( i.isEmpty() ) i = " "; // Force property save
    m_inputStr = i;
    QStringList inputList = i.split(",");
    inputList.removeAll(" ");

    int size = inputList.size();
    IoComponent::setNumOuts( size, "I" );

    for( int i=0; i<size; ++i )
        m_outPin[i]->setLabelText( inputList.at(i) );

    updtOutPins();
    resizeVectors();
}

void TestUnit::setOutputs( QString o )
{
    if( o.isEmpty() ) o = " "; // Force property save
    m_outputStr = o;
    QStringList outputList = o.split(",");
    outputList.removeAll(" ");

    int size = outputList.size();
    IoComponent::setNumInps( size, "O" );

    for( int i=0; i<size; ++i )
        m_inpPin[i]->setLabelText( outputList.at(i) );

    updtInPins();
    resizeVectors();
}

void TestUnit::resizeVectors() // Vector size is nº of combinations, bits in uint value is an output state
{
    m_steps = pow( 2, m_outPin.size() ); // Output pins are the inputs of the tested device
    m_samples.clear();
    m_samples.resize( m_steps, 0 );

    if( m_truthT.size() == (uint)m_steps ) return;
    m_truthT.clear();
    m_truthT.resize( m_steps, 0 );
}

void TestUnit::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* tableAction = menu->addAction( m_theme->icon(":/list.svg"),tr("Show Table") );
    QObject::connect( tableAction, &QAction::triggered, [=](){ createTable(); } );

    menu->addSeparator();

    Component::contextMenu( event, menu );
}

