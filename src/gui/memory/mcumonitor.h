#ifndef MCUMONITOR_H
#define MCUMONITOR_H

#include <QDialog>

#include "ui_mcumonitor.h"

class BaseProcessor;
class MemTable;
class RamTable;

class MCUMonitor : public QDialog, private Ui::McuMonitor
{
    Q_OBJECT
    
    public:
        MCUMonitor( QWidget* parent=0, BaseProcessor* mcu=0 );

        void updateStep();

    public slots:
        void ramDataChanged( int address, int val );
        void flashDataChanged( int address, int val );
        void eepromDataChanged( int address, int val );
        void tabChanged( int index );

    private:
        void updateTable( int index );
        BaseProcessor* m_processor;

        RamTable* m_ramTable;
        MemTable* m_ramMonitor;
        MemTable* m_flashMonitor;
        MemTable* m_romMonitor;
};


#endif
