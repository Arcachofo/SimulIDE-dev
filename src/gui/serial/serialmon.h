/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SERIALMON_H
#define SERIALMON_H

#include <QDialog>

#include "ui_serialmon.h"
#include "outpaneltext.h"
#include "updatable.h"

class UsartModule;

class SerialMonitor : public QDialog, private Ui::SerialMonitor, public Updatable
{
    Q_OBJECT

    public:
        SerialMonitor( QWidget* parent, UsartModule* usart );

        virtual void updateStep() override;

        void printIn( int value );
        void printOut( int value );

    public slots:
        void on_text_returnPressed();
        void on_value_returnPressed();
        void on_valueButton_clicked();
        void on_asciiButton_clicked();
        void on_addCrButton_clicked() { m_addCR = addCrButton->isChecked(); }
        void on_clearIn_clicked() { m_uartInPanel.clear(); }
        void on_clearOut_clicked() { m_uartOutPanel.clear(); }

    protected:
        void closeEvent( QCloseEvent* event );

    private:
        OutPanelText m_uartInPanel;
        OutPanelText m_uartOutPanel;

        UsartModule* m_usart;

        bool m_printASCII;
        bool m_addCR;

        QByteArray m_outBuffer;
};

#endif
