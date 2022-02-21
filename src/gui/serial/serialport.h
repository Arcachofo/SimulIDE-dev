/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>

#include "component.h"
#include "e-element.h"
#include "usartmodule.h"

class LibraryItem;
class QPushButton;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT SerialPort : public Component, public UsartModule, public eElement
{
    Q_OBJECT
    /*

    Q_PROPERTY( QSerialPort::Parity   Parity   READ parity   WRITE setParity    DESIGNABLE true  USER true )
    Q_PROPERTY( QSerialPort::FlowControl FlowControl READ flowControl WRITE setFlowControl  DESIGNABLE true  USER true )
*/
    public:

        SerialPort( QObject* parent, QString type, QString id );
        ~SerialPort();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void runEvent() override;

        QString port(){return m_portName;}
        void setPort( QString name ){ m_portName = name; update();}

        int dataBits(){ return m_dataBits; }
        void setDataBits( int db ){ m_dataBits = db; }

        int parity() { return (int)m_parity; }
        void setParity( int par ) { m_parity = (parity_t)par; }

        int stopBits() { return m_stopBits; }
        void setStopBits( int sb ) { m_stopBits = sb; }

        QSerialPort::FlowControl flowControl() { return m_flowControl; }
        void setFlowControl( QSerialPort::FlowControl fc ) { m_flowControl = fc; }

        virtual void byteReceived( uint8_t byte ) override;
        virtual void frameSent( uint8_t data ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void slotClose();
        void slotOpenTerm();

    private slots:
        void readData();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

    private:
        void open();
        void close();

        QPushButton* m_button;
        QGraphicsProxyWidget* m_proxy;

        QSerialPort* m_serial;

        bool m_receiving;
        bool m_sending;

        QByteArray m_serData;
        QByteArray m_uartData;

        QString m_portName;

        /*QSerialPort::DataBits    m_dataBits;
        QSerialPort::Parity      m_parity;
        QSerialPort::StopBits    m_stopBits;*/
        QSerialPort::FlowControl m_flowControl;
};

#endif

