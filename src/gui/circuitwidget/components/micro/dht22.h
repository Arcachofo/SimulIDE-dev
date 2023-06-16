/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DHT22_H
#define DHT22_H

#include "e-element.h"
#include "component.h"

class LibraryItem;
class QPushButton;
class QGraphicsProxyWidget;
class IoPin;

class MAINMODULE_EXPORT Dht22 : public Component , public eElement
{
    public:
        Dht22( QObject* parent, QString type, QString id );
        ~Dht22();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        void setModel( QString model);
        QString model();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double humidInc() { return m_humiInc; }
        void setHumidInc( double inc ) { m_humiInc = trim( inc ); }

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void onbuttonclicked();
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        void calcData();
        double trim( double data );

        bool m_DHT22;
        bool m_lastIn;
        bool m_set;

        uint64_t m_lastTime;
        uint64_t m_start;
        uint64_t m_data;
        uint64_t m_bit;

        double m_temp;
        double m_humi;
        double m_tempInc;
        double m_humiInc;

        int m_outStep;
        int m_bitStep;

        QFont m_font;

        QPushButton* m_button;
        QGraphicsProxyWidget* m_proxy;

        IoPin* m_inpin;
};

#endif
