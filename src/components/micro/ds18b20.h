/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DS18B20_H
#define DS18B20_H

#include <QFont>

#include "onewire.h"
#include "component.h"

class LibraryItem;
class IoPin;

class Ds18b20 : public Component , public OneWire
{
    public:
        Ds18b20( QString type, QString id );
        ~Ds18b20();

        enum dsCommand_t {
            DS_CONVERT = 0x44,
            DS_WR_SCRP = 0x4E,
            DS_RD_SCRP = 0xBE,
            DS_CP_SCRP = 0x48,
            DS_REC_E2  = 0xB8,
            DS_RD_POW  = 0xB4
        };

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = inc; }

        double temp() { return m_temp; }
        void setTemp( double t );

        void updateStep() override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    public slots:
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        void readByte() override;
        void funCommand( uint8_t cmd ) override;

        // Function commands
        void convertTemp();     // 44h
        void writeScratchpad(); // 4Eh
        void readScratchpad();  // BEh
        void copyScratchpad();  // 48h
        void recallE2();        // B8h
        void readPowerSupply(); // B4h

        double m_temp;
        double m_tempInc;

        QFont m_font;

        int m_resolution; // 9-12 bits
        uint8_t m_scratchpad[9];

        int8_t  m_TH;  // TH register, alarm trigger register, can be stored in internal EEPROM
        int8_t  m_TL;  // TL register, alarm trigger register, can be stored in internal EEPROM
        uint8_t m_CFG; // Config register, can be stored in internal EEPROM
};

#endif
