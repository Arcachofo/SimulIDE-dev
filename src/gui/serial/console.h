#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>

#include "updatable.h"

class CoreBase;

class Console : public QPlainTextEdit, public Updatable
{
    public:
        Console( CoreBase* cpu, QWidget* parent=nullptr );

        virtual void updateStep() override;

        void appendText( QString text );
        void appendLine( QString line );

    protected:
        void keyPressEvent(QKeyEvent* e) override;
        void mousePressEvent(QMouseEvent* e) override;
        void mouseDoubleClickEvent(QMouseEvent* e) override;
        void contextMenuEvent(QContextMenuEvent* e) override;

    private:
        CoreBase* m_cpu;

        bool m_sendCommand;

        QString m_command;
        QString m_buffer;
};
#endif
