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

#ifndef SERIALMON_H
#define SERIALMON_H

#include <QDialog>

#include "ui_serialmon.h"
#include "outpaneltext.h"

class McuInterface;

class SerialMonitor : public QDialog, private Ui::SerialMonitor
{
    Q_OBJECT

    public:
        SerialMonitor( QWidget* parent, McuInterface* mcu, int uartNum=1 );

        void printIn( int value );
        void printOut( int value );

    public slots:
        void on_text_returnPressed();
        void on_value_returnPressed();
        void on_valueButton_clicked();
        void on_asciiButton_clicked();
        void on_addCR_clicked() { m_addCR = addCrButton->isChecked(); }
        void on_clearIn_clicked() { m_uartInPanel.clear(); }
        void on_clearOut_clicked() { m_uartOutPanel.clear(); }

    private:
        OutPanelText m_uartInPanel;
        OutPanelText m_uartOutPanel;

        McuInterface* m_processor;

        int m_uartNum;

        bool m_printASCII;
        bool m_addCR;
};

#endif
