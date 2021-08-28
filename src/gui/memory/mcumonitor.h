#ifndef MCUMONITOR_H
#define MCUMONITOR_H

#include <QDialog>

#include "ui_mcumonitor.h"

class McuInterface;
class MemTable;
class RamTable;
class QTableWidget;

class MCUMonitor : public QDialog, private Ui::McuMonitor
{
    Q_OBJECT
    
    public:
        MCUMonitor( QWidget* parent=0, McuInterface* mcu=0 );

        void updateStep();

    public slots:
        void ramDataChanged( int address, int val );
        void flashDataChanged( int address, int val );
        void eepromDataChanged( int address, int val );
        void tabChanged(int);
        void on_byteButton_toggled( bool byte );
        void on_jumpButton_toggled( bool jump );

    private:
        McuInterface* m_processor;

        RamTable* m_ramTable;
        MemTable* m_ramMonitor;
        MemTable* m_flashMonitor;
        MemTable* m_romMonitor;

        QTableWidget* m_status;
        QTableWidget* m_pc;

        bool m_jumpToAddress;
};


#endif
