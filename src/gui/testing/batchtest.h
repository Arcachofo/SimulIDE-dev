/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BATCHTEST_H
#define BATCHTEST_H

#include <QDir>

class Component;

class BatchTest
{
    public:

        static void doBatchTest( QString folder );

        static bool isRunning() { return m_running; }
        static void addTestUnit( Component* c );
        static void testCompleted( Component* c, bool ok );

        static void checkFinished();

    private:
        static void prepareTest( QDir dir );
        static void runNextCircuit();

        static bool m_running;

        static QString m_currentFile;

        static QStringList m_failedTests;
        static QStringList m_circFiles;
        static QList<Component*> m_testUnits;
};

#endif
