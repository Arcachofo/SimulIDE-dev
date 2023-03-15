/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUMONITOR_H
#define MCUMONITOR_H

#include <QDialog>
#include <QTableWidget>

#include "ui_mcumonitor.h"

class eMcu;
class MemTable;
class RamTable;
class Watcher;

class MCUMonitor : public QDialog, private Ui::McuMonitor
{
    Q_OBJECT
    
    public:
        MCUMonitor( QWidget* parent=0, eMcu* mcu=0 );

        void updateStep();
        void updateRamTable();

    public slots:
        void ramDataChanged( int address, int val );
        void flashDataChanged( int address, int val );
        void eepromDataChanged( int address, int val );
        void tabChanged(int);
        void on_byteButton_toggled( bool byte );
        void on_jumpButton_toggled( bool jump );

    private:
        void createStatusPC();

        eMcu* m_processor;

        uint8_t* m_statusReg;  // STATUS register

        Watcher* m_cpuTable;
        RamTable* m_ramTable;
        MemTable* m_ramMonitor;
        MemTable* m_flashMonitor;
        MemTable* m_romMonitor;

        QTableWidget m_status;
        QTableWidget m_pc;

        bool m_jumpToAddress;
};


#endif
