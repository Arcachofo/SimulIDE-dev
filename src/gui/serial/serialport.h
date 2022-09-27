/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        void setSerialMon( bool s );

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

