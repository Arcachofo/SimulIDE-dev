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
        SerialMonitor( QWidget* parent, UsartModule* usart, bool send=false );

        virtual void updateStep() override;

        void printIn( int value );
        void printOut( int value );

        void activateSend();

    public slots:
        void on_text_returnPressed();
        void on_value_returnPressed();
        void on_printBox_currentIndexChanged( int index );
        void on_addCrButton_clicked() { m_addCR = addCrButton->isChecked(); }
        void on_pauseButton_clicked();
        void on_clearIn_clicked() { m_uartInPanel.clear(); }
        void on_clearOut_clicked() { m_uartOutPanel.clear(); }

    protected:
        void closeEvent( QCloseEvent* event ) override;

    private:
        QString valToString( int val );

        OutPanelText m_uartInPanel;
        OutPanelText m_uartOutPanel;

        UsartModule* m_usart;

        int  m_printMode;
        bool m_addCR;
        bool m_paused;

        QByteArray m_outBuffer;
};

#endif
