/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QTableWidget>
#include <QHeaderView>
#include <math.h>

#include "testunit.h"
#include "itemlibrary.h"
#include "simulator.h"
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
        "3to2g.png",
        "TestUnit",
        TestUnit::construct );
}

TestUnit::TestUnit( QString type, QString id )
        : IoComponent( type, id )
        , eElement( id )
{
    m_width  = 4;
    m_height = 4;

    m_interval = 100*1e3; // 100 ns
    m_truthTable = nullptr;

    setInputs("I0,I1");
    setOutputs("O");

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new StrProp<TestUnit>("Inputs", tr("Inputs"),""
                             , this, &TestUnit::inputs, &TestUnit::setInputs, propNoCopy ),

        new StrProp<TestUnit>("Outputs", tr("Outputs"),""
                             , this, &TestUnit::outputs, &TestUnit::setOutputs, propNoCopy ),
    },0} );
}
TestUnit::~TestUnit()
{
    if( m_truthTable )
    {
        m_truthTable->clear();
        delete m_truthTable;
    }
}

void TestUnit::stamp()
{
    IoComponent::initState();

    m_read = false;
    m_changed = false;

    m_steps = pow( 2, m_outPin.size() );
    m_values.clear();
    m_values.resize( m_steps );

    Simulator::self()->addEvent( m_interval, this );
}

void TestUnit::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    createTable();
}

void TestUnit::runEvent()
{
    if( m_read )
    {
        m_read = false;

        uint inputVal = 0;
        for( uint i=0; i<m_inPin.size(); ++i )
            if( m_inPin[i]->getInpState() ) inputVal |= 1<<i;

        m_values[m_outValue] = inputVal;

        if( ++m_outValue < (uint)m_steps )
            Simulator::self()->addEvent( m_interval, this );
        else
            m_changed = true;
    }
    else
    {
        m_read = true;
        for( uint i=0; i<m_outPin.size(); ++i )
        {
            bool state = m_outValue & (1<<i);
            m_outPin[i]->setOutState( state );
        }

        Simulator::self()->addEvent( m_interval, this );
    }
}

void TestUnit::createTable()
{
    int inputs  = m_outPin.size();
    int columns = inputs+ m_inPin.size();

    if( !m_truthTable )
        m_truthTable = new QTableWidget( m_steps, columns );

    m_truthTable->clear();
    m_truthTable->verticalHeader()->setVisible( false );

    QStringList header = m_inputStr.split(",");
    header.append( m_outputStr.split(",") );
    m_truthTable->setHorizontalHeaderLabels( header );
    m_truthTable->horizontalHeader()->setDefaultSectionSize( 10 );

    for( int row=0; row<m_steps; ++row )
    {
        uint values = m_values[row];
        for( int col=0; col<columns; ++col )
        {
            int value = 0;
            if( col < inputs ) value = (   row & 1<<col);
            else               value = (values & 1<<(col-inputs));
            QString valStr = value ? "H" : "L";
            m_truthTable->setItem( row, col, new QTableWidgetItem( valStr ) );
        }
    }
    //for( int col=0; col<columns; ++col )
    //    m_truthTable->horizontalHeader()->resizeSection( col, 10 );
    //resizeColumnToContents( col );//setColumnWidth( col, 30 );

    m_truthTable->show();
}

void TestUnit::loadTest()
{

}

void TestUnit::setTest( QString t )
{

}

void TestUnit::setInputs( QString i )
{
    m_inputStr = i;
    QStringList inputList = i.split(",");

    int size = inputList.size();
    IoComponent::setNumOuts( size, "I" );

    for( int i=0; i<size; ++i )
        m_outPin[i]->setLabelText( inputList.at(i) );
}

void TestUnit::setOutputs( QString o )
{
    m_outputStr = o;
    QStringList outputList = o.split(",");

    int size = outputList.size();
    IoComponent::setNumInps( size, "O" );

    for( int i=0; i<size; ++i )
        m_inPin[i]->setLabelText( outputList.at(i) );
}
