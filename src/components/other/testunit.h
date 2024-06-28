/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TESTUNIT_H
#define TESTUNIT_H

#include "iocomponent.h"
#include "e-element.h"

class LibraryItem;
class QTableWidget;

class TestUnit: public IoComponent, public eElement
{
    public:
        TestUnit( QString type, QString id );
        ~TestUnit();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void runEvent() override;

        QString inputs() { return m_inputStr; }
        void setInputs( QString i );

        QString outputs() { return m_outputStr; }
        void setOutputs( QString o );

        QString test() { return m_test; }
        void setTest( QString t );

        void loadTest();

    private:
        void createTable();

        uint64_t m_interval;

        bool m_read;
        int m_steps;

        std::vector<uint> m_values;

        QString m_test;
        QString m_inputStr;
        QString m_outputStr;

        QTableWidget* m_truthTable;
};
#endif
