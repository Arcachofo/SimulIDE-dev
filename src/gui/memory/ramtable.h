/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RAMTABLE_H
#define RAMTABLE_H

#include <QWidget>

#include "ui_ramtable.h"

class eMcu;
class BaseDebugger;
class QStandardItemModel;
class QSplitter;

class RamTable : public QWidget, private Ui::RamTable
{
    Q_OBJECT

    public:
        RamTable( QWidget* parent=0, eMcu* processor=NULL, bool cpuMonitor=false );

        void updateValues();

        //void setItemValue( int col, QString v ) { table->item( m_currentRow, col )->setData( 0, v ); }
        //void setItemValue( int col, int32_t v ) { table->item( m_currentRow, col )->setData( 0, v ); }
        //void setItemValue( int col, float v ) { table->item( m_currentRow, col )->setData( 0, v ); }

        void setRegisters( QStringList regs );
        void addRegister( QString name, QString type );

        void setDebugger( BaseDebugger* deb ) { m_debugger = deb; }
        void remDebugger( BaseDebugger* deb ) { if( m_debugger == deb ) m_debugger = NULL; }

        void setVariables( QStringList vars );
        void addVariable( QString name, int address, QString type );
        void addVariable( QString name, QString type );

        void loadVarSet( QStringList varSet );
        QStringList getVarSet();
        uint16_t getCurrentAddr();

        void updateItems();

        QSplitter* getSplitter() { return splitter; }

    public slots:
        void RegDoubleClick( const QModelIndex& index );
        void VarDoubleClick( const QModelIndex& index );
        void clearSelected();
        void clearTable();
        void loadVarSet();
        void saveVarSet();

    private slots:
        void addToWatch( QTableWidgetItem* );
        void slotContextMenu( const QPoint& );

    private:
        //void updateRamValue( QString name );
        void setAddress( int r, QString a );
        void setName( int r, QString n );
        void setValue( int r, QString v );
        void setType( int r, QString t );

        eMcu* m_processor;
        BaseDebugger*  m_debugger;

        QStandardItemModel* m_registerModel;
        QStandardItemModel* m_variableModel;

        QHash<int, QString> watchList;

        QHash<QString, QString> m_typeTable;
        QHash<QString, uint16_t> m_varsTable;
        QStringList m_varNames;
        QStringList m_regNames;

        bool m_loadingVars;
        bool m_cpuMonitor;

        int m_numRegs;
        int m_currentRow;
};
#endif // RAMTABLE_H
