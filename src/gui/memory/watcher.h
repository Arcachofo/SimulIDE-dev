/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef WATCHER_H
#define WATCHER_H

#include <QWidget>

#include "ui_watcher.h"

class eMcu;
class ValueWidget;
class QStandardItemModel;
class ScriptCpu;
class Console;

class Watcher : public QWidget, private Ui::Watcher
{
    Q_OBJECT

    public:
        Watcher( QWidget* parent=0, eMcu* processor=NULL );

        void updateValues();

        void setRegisters( QStringList regs );
        void addRegister( QString name, QString type );

        void setVariables( QStringList vars );
        void addVariable( QString name, QString type );

        void loadVarSet( QStringList varSet );
        QStringList getVarSet();

        void addConsole( ScriptCpu* s );
        Console* console() { return m_console; }

    public slots:
        void RegDoubleClick( const QModelIndex& index );
        void VarDoubleClick( const QModelIndex& index );

    private:
        void addHeader();

        bool m_header;

        eMcu* m_processor;
        Console* m_console;

        QStandardItemModel* m_registerModel;
        QStandardItemModel* m_variableModel;

        QHash<QString, QString> m_typeTable;
        QHash<QString, ValueWidget*> m_values;

        QBoxLayout* m_valuesLayout;
};

#endif
