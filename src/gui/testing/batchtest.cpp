/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QTimer>

#include "batchtest.h"
#include "component.h"
#include "circuitwidget.h"

bool BatchTest::m_running = false;
QString BatchTest::m_currentFile;
QStringList BatchTest::m_failedTests;
QStringList BatchTest::m_circFiles;
QList<Component*> BatchTest::m_testUnits;

void BatchTest::doBatchTest( QString folder )
{
    prepareTest( QDir(folder) );
    m_failedTests.clear();
    m_circFiles.clear();
    m_running = true;
    runNextCircuit();
}

void BatchTest::prepareTest( QDir baseDir )
{
    QStringList circList = baseDir.entryList( {"*.sim1"}, QDir::Files );
    m_circFiles.append( circList );

    QStringList dirList = baseDir.entryList( {"*"}, QDir::Dirs );
    for( QString dir : dirList )
    {
        if( dir == "." || dir == "..") continue;
        if( !baseDir.cd( dir ) ) continue;

        prepareTest( baseDir );
        baseDir.cd("..");
    }
}

void BatchTest::runNextCircuit()
{
    CircuitWidget::self()->powerCircOff();

    if( m_circFiles.isEmpty() )
    {
        m_running = false;
        return;
    }
    m_currentFile = m_circFiles.takeFirst();

    CircuitWidget::self()->loadCirc( m_currentFile );

    m_testUnits.clear();
    CircuitWidget::self()->powerCircOn();

    if( m_testUnits.isEmpty() ) // No test units in this Circuit
    {
        QTimer::singleShot( 50, [=](){ runNextCircuit(); } );
    }
}

void BatchTest::addTestUnit( Component* c )
{
    if( !m_testUnits.contains( c ) ) m_testUnits.append( c );
}

void BatchTest::testCompleted( Component* c, bool ok ) // A test unit completed (could be more in this Circuit)
{
    m_testUnits.removeAll( c );

    if( !ok ){  // Test failed
        if( !m_failedTests.contains( m_currentFile) ) m_failedTests.append( m_currentFile );
    }
    if( m_testUnits.isEmpty() ) runNextCircuit(); // All test units completed for this Circuit
}

