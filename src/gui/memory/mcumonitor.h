/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef MCUMONITOR_H
#define MCUMONITOR_H

#include <QDialog>
#include <QTableWidget>

#include "ui_mcumonitor.h"

class eMcu;
class MemTable;
class RamTable;

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

        RamTable* m_cpuTable;
        RamTable* m_ramTable;
        MemTable* m_ramMonitor;
        MemTable* m_flashMonitor;
        MemTable* m_romMonitor;

        QTableWidget m_status;
        QTableWidget m_pc;

        bool m_jumpToAddress;
};


#endif
